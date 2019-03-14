const _ = require('lodash');
const { EventEmitter } = require('events');
const fs = require('./async-fs');
const logger = require('./logger');
const lt = require('@porla/libtorrent');
const path = require('path');
const proxyType = require('./proxyType');
const Torrent = require('./torrent');

class Session extends EventEmitter {
    constructor(options) {
        super();

        this._exiting        = false;
        this._mutedTorrents  = new Set();
        this._options        = options || {};
        this._options.dht    = options.dht || {};
        this._timers         = {};
        this._torrents       = new Map();
        this._wrappedSession = null;
    }

    torrents() {
        return this._torrents.values();
    }

    addTorrent(fileOrBuffer, options) {
        options          = options          || {};
        options.savePath = options.savePath || this._options.savePath;

        if (!options.savePath || options.savePath === '') {
            throw new Error('No save path passed to function and no save path set in Porla options.');
        }

        const params     = new lt.add_torrent_params();
        params.save_path = options.savePath;
        params.ti        = new lt.torrent_info(fileOrBuffer);

        this._wrappedSession.async_add_torrent(params);

        return params.ti.info_hash();
    }

    async load() {
        const sessionSettings = {
            alert_mask: lt.alert.all_categories,
            anonymous_mode: !!this._options.anonymousMode,
            enable_dht: false, // disabled by default
            enable_lsd: true,
            in_enc_policy: 1, // enabled
            out_enc_policy: 1, // enabled
            peer_fingerprint: lt.generate_fingerprint('PO', 0, 0, 1),
            stop_tracker_timeout: 1,
            user_agent: 'Porla/alpha libtorrent/1.2.0'
        };

        // Enable DHT if it is not explicitly disabled
        // enabled:  dht: {}
        // enabled:  dht: { bootstrapNodes: [] }
        // enabled:  dht: { enabled: true }
        // disabled: dht: { enabled: false }
        let dhtEnabled = true;

        if ('enabled' in this._options.dht
            && typeof this._options.dht.enabled === 'boolean'
            && this._options.dht.enabled === false) {
            dhtEnabled = false;
        }

        if (dhtEnabled) {
            let bootstrapNodes = [
                [ 'router.bittorrent.com', 6881 ],
                [ 'router.utorrent.com', 6881 ],
                [ 'dht.transmissionbt.com', 6881 ],
                [ 'dht.aelitis.com', 6881 ]
            ];

            if (Array.isArray(this._options.dht.bootstrapNodes)) {
                bootstrapNodes = this._options.dht.bootstrapNodes;
            }

            sessionSettings.enable_dht = true;
            sessionSettings.dht_bootstrap_nodes = bootstrapNodes
                .map(val => `${val[0]}:${val[1]}`)
                .join(',');
        }

        // If proxy
        if (this._options.proxy) {
            if (!this._options.proxy.type) {
                throw new Error('Missing proxy.type');
            }

            if (!this._options.proxy.host) {
                throw new Error('Missing proxy.host');
            }

            if (!this._options.proxy.port) {
                throw new Error('Missing proxy.port');
            }

            switch (this._options.proxy.type) {
                case proxyType.SOCKS5:
                    logger.info('Setting up SOCKS5 proxy');

                    sessionSettings.proxy_type     = this._options.proxy.type;
                    sessionSettings.proxy_hostname = this._options.proxy.host;
                    sessionSettings.proxy_port     = this._options.proxy.port;

                    break;

                default:
                    throw new Error('Invalid proxy type: ' + this._options.proxy.type);
            }
        }

        // Listen interfaces
        if (Array.isArray(this._options.listenInterfaces)) {
            sessionSettings.listen_interfaces = this._optons.listenInterfaces
                .map(val => `${val[0]}:${val[1]}`)
                .join(',');
        }

        // Require encryption, perhaps.
        if (typeof this._options.requireEncryption === 'boolean') {
            if (this._options.requireEncryption) {
                sessionSettings.in_enc_policy  = 0; // forced
                sessionSettings.out_enc_policy = 0; // forced
            }
        }

        this._wrappedSession = new lt.session(sessionSettings);

        await this.loadState();
        await this.loadTorrents();

        this.readAlerts();

        this._timers.postUpdates = setInterval(() => {
            this._wrappedSession.post_dht_stats();
            this._wrappedSession.post_session_stats();
            this._wrappedSession.post_torrent_updates();
        }, 1000);
    }

    async loadState() {
        const stateFile = path.join(this._options.dataPath, 'state.dat');

        logger.info(`Loading session state from ${stateFile}`);

        try {
            const buffer = await fs.readFile(stateFile);
            this._wrappedSession.load_state(buffer);
        } catch (err) {
            // We accept the ENOENT error since the data folder or state.dat
            // file might not exist yet.
            if (err.code !== 'ENOENT') {
                logger.error(`Unknown error when loading state file: ${err}`);
            }
        }
    }

    async loadTorrents() {
        let files = [];

        try {
            files = await fs.readdir(this._options.dataPath);
        } catch (err) {
            // We accept the ENOENT error since the data folder or state.dat
            // file might not exist yet.
            if (err.code !== 'ENOENT') {
                logger.error(`Unknown error when loading torrent files: ${err}`);
                return;
            }
        }

        const dats  = files.filter(f => path.extname(f) === '.json')
                           .filter(f => /[0-9a-f]{40}/.test(path.basename(f, '.json')));

        const items = [];

        logger.info(`Loading ${dats.length} torrent(s) from ${this._options.dataPath}`);

        for (const dat of dats) {
            const p       = path.join(this._options.dataPath, dat);
            const buf     = await fs.readFile(p);
            const decoded = JSON.parse(buf);

            if (decoded !== Object(decoded)) {
                logger.warn(`.json file (${p}) not an object`);
                // TODO: err for dat file. should log warn and remove
                // dat file and corresponding torrent file (if it exists)
                continue;
            }

            if (!('resume' in decoded)) {
                logger.warn(`.json file (${p}) does not contain a resume key`);
                continue;
            }

            const resumeBuffer = Buffer.from(decoded.resume, 'base64');
            let queuePosition  = Number.MAX_VALUE;

            if ('queuePosition' in decoded) {
                queuePosition = Number(decoded.queuePosition);
            }

            items.push({
                infoHash: path.basename(dat, '.json'),
                params: lt.read_resume_data(resumeBuffer),
                queuePosition: queuePosition,
            });
        }

        const sortedItems = _.sortBy(items, x => x.queuePosition);

        for (const item of sortedItems) {
            const tf = path.join('data', item.infoHash + '.torrent');

            try {
                item.params.ti = new lt.torrent_info(tf);
            } catch (err) {
                logger.error(`Error when parsing torrent file (${tf}): ${err}`);
                continue;
            }

            this._mutedTorrents.add(item.infoHash);
            this._wrappedSession.async_add_torrent(item.params);
        }
    }

    async readAlerts() {
        do {
            const foundAlerts = await this.waitForAlerts(100);

            if (!foundAlerts) {
                continue;
            }

            const alerts = this._wrappedSession.pop_alerts();

            for (const alert of alerts) {
                switch (alert.what) {
                    case 'add_torrent': {
                        if (alert.error) {
                            logger.error(`Error when adding torrent: ${alert.error.message}`);
                            continue;
                        }

                        const status = alert.handle.status();

                        logger.info(`Torrent added (${status.info_hash})`);

                        // Check if this torrent is a muted torrent - i.e it
                        // was added as a part of the load function.
                        if (this._mutedTorrents.has(status.info_hash)) {
                            this._mutedTorrents.delete(status.info_hash);
                        } else {
                            const ct = new lt.create_torrent(status.torrent_file);
                            const file = ct.generate();
                            const buffer = lt.bencode(file);
                            const p = path.join('data', status.info_hash + '.torrent');

                            try {
                                await fs.mkdir('data');
                            } catch (err) {
                                // TODO: Only catch when dir already exists
                            }

                            await fs.writeFile(p, buffer);
                        }

                        if (!this._torrents.has(status.info_hash)) {
                            this._torrents.set(status.info_hash, new Torrent(alert.handle));
                        }

                        this.emit('torrent.added', this._torrents.get(status.info_hash));

                        } break;

                    case 'state_update': {
                        const torrentsStats = alert.status.reduce((acc, curr) => {
                            acc.downloadPayloadRate += curr.download_payload_rate;
                            acc.uploadPayloadRate += curr.upload_payload_rate;
                            return acc;
                        }, {
                            downloadPayloadRate: 0,
                            uploadPayloadRate: 0
                        });

                        // TODO this.emit('torrents.statistics', { stats: torrentsStats });

                        const updatedTorrents = [];

                        for (const status of alert.status) {
                            const updatedTorrent = this._torrents.get(status.info_hash);
                            updatedTorrent._status = status;
                            updatedTorrents.push(updatedTorrent);
                        }

                        this.emit('torrents.updated', updatedTorrents);

                        } break;

                    case 'torrent_finished': {
                        const status   = alert.handle.status();
                        const infoHash = status.info_hash;
                        const torrent  = this._torrents.get(infoHash);

                        // Update the status for this torrent while we have it.
                        torrent._status = status;

                        // Only emit this event for torrents that we actually
                        // downloaded any data for.
                        if (status.total_payload_download > 0) {
                            this.emit('torrent.finished', torrent);
                        }

                        } break;

                    case 'torrent_paused': {
                        const pausedTorrent = this._torrents.get(alert.handle.info_hash());
                        pausedTorrent.emit('paused');
                        this.emit('torrent.paused', pausedTorrent);
                        } break;

                    case 'torrent_removed': {
                        const removedTorrent = this._torrents.get(alert.info_hash);
                        removedTorrent.emit('removed');
                        this._torrents.delete(alert.info_hash);
                        this.emit('torrent.removed', infoHash);
                        } break;
                }
            }
        } while (!this._exiting);

        this.emit('_internal.readAlerts.finished');
    }

    unload() {
        return new Promise(resolve => {
            logger.debug('Starting to unload session');

            this.once('_internal.readAlerts.finished', async () => {
                logger.debug('Unloading Porla...');

                clearInterval(this._timers.postUpdates);

                const state = this._wrappedSession.save_state_buf();

                try {
                    await fs.mkdir(this._options.dataPath);
                } catch (err) { }

                try {
                    await fs.writeFile(path.join(this._options.dataPath, 'state.dat'), state);
                } catch (err) {
                    logger.error('Failed to save session state');
                }

                logger.debug('Pausing session');

                this._wrappedSession.pause();

                let numOutstandingResumeData = 0;
                const tempStatus = this._wrappedSession.get_torrent_status();

                for (const st of tempStatus) {
                    if (!st.handle.is_valid()
                        || !st.has_metadata
                        || !st.need_save_resume) {
                        continue;
                    }

                    st.handle.save_resume_data();
                    numOutstandingResumeData += 1;
                }

                logger.info(`Saving resume data for ${numOutstandingResumeData} torrents.`);

                while (numOutstandingResumeData > 0) {
                    const foundAlerts = await this.waitForAlerts(1000);
                    if (!foundAlerts) { continue; }

                    const alerts = this._wrappedSession.pop_alerts();

                    for (const alert of alerts) {
                        if (alert.what === 'torrent_paused') {
                            continue;
                        }

                        if (alert.what === 'save_resume_data_failed') {
                            numOutstandingResumeData -= 1;
                            continue;
                        }

                        if (alert.what !== 'save_resume_data') {
                            continue;
                        }

                        numOutstandingResumeData -= 1;

                        const statePath = path.join(this._options.dataPath, alert.handle.info_hash() + '.json');
                        const status = alert.handle.status();

                        const state = {
                            resume: lt.write_resume_data_buf(alert.params).toString('base64'),
                            queuePosition: status.queue_position
                        };

                        await fs.writeFile(statePath, JSON.stringify(state, null, 2));
                    }
                }

                return resolve();
            });

            this._exiting = true;
        });
    }

    waitForAlerts(timeout) {
        return new Promise((resolve, reject) => {
            this._wrappedSession.wait_for_alert(timeout, (err, result) => {
                if (err) {
                    return reject(err);
                }

                return resolve(result);
            });
        });
    }
}

module.exports = Session;
