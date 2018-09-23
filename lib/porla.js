const _ = require('lodash');
const fs = require('./async-fs');
const logger = require('./logger');
const lt = require('@porla/libtorrent');
const path = require('path');
const Plugin = require('./plugin');
const process = require('process');
const Subscription = require('./subscription');
const Torrent = require('./torrent');
const { URL } = require('url');

class Monitor {
    /**
     * Creates a new Monitor class.
     * @param {URL} url
     */
    constructor(url, options) {
        this.url = url;
        this.options = options || {};
    }

    /**
     * Gets the files monitored by this monitor instance.
     */
    getFiles() { throw new Error('Should be overridden in subclass.') }
}

class FileMonitor extends Monitor {
    constructor(url, options) {
        super(url, options);
    }

    async getFiles() {
        const files = await fs.readdir(this.url.pathname);
        const filePaths = files.map(x => path.join(this.url.pathname, x));
        const results = [];

        for (const res of filePaths) {
            if (path.extname(res) !== '.torrent') {
                continue;
            }

            const buffer = await fs.readFile(res);

            results.push(buffer);

            if (this.options.remove) {
                await fs.unlink(res);
            }
        }

        return results;
    }
}

class Porla {
    /**
     * Constructs a new Porla instance.
     * @param {Object} options The options to configure the instance with.
     * @param {string} options.dataPath The path where all session related data is saved.
     * @param {Object} options.logging Logging options.
     * @param {Array} options.logging.outputs User-defined logging outputs.
     * @param {Array<Plugin>} options.plugins A list of plugins to associate with Porla.
     * @param {string} options.savePath The default save path - i.e where to save downloads.
     */
    constructor(options) {
        options = options || {};

        // Set default options
        options.dataPath = options.dataPath || path.join(process.cwd(), 'data');
        options.savePath = options.savePath || process.cwd();

        // Logging
        if (options.logging && options.logging.outputs) {
            logger.outputs.push(...options.logging.outputs);
        }

        this._exiting = false;
        this._monitors = [];
        this._mutedTorrents = [];
        this._options = options;
        this._plugins = options.plugins || [];
        this._session = null;

        // Map session metrics to an object indexed by the name.
        const tmp = lt.session_stats_metrics();
        this._sessionMetrics = tmp.reduce((acc, curr) => {
            acc[curr.name] = {
                index: curr.value_index,
                type: curr.type
            };
            return acc;
        }, {});

        this._subscriptions = {};
        this._timers = {};
        this._torrents = new Map();

        process.nextTick(this._load.bind(this));
        process.on('SIGINT', () => { this._exiting = true; });
    }

    get log() {
        return logger;
    }

    /**
     * @type {Array<Torrent>}
     */
    get torrents() {
        return this._torrents.values();
    }

    async emit(eventName, parameter) {
        if (!(eventName in this._subscriptions)) {
            return Promise.resolve();
        }

        for (const subscription of this._subscriptions[eventName]) {
            await subscription.execute(parameter);
        }
    }

    /**
     * Monitor a URL. Depending on plugins, you might specify a protocol
     * as well. For example, with the right plugin, `ftp://user:pass@127.0.0.1/files`
     * might be valid. The default is to monitor the local file system, so any
     * URL with no protocol will default to `file://`.
     * @param {string} url
     */
    monitor(url, options) {
        if (!hasProtocol(url)) {
            url = `file://${url}`;
        }

        const u = new URL(url);
        const monitor = new FileMonitor(u, options);

        this._monitors.push(monitor);
    }

    remove(torrent) {
        this._session.remove_torrent(torrent._handle);
    }

    /**
     * 
     * @param {string} eventName 
     * @param {Array} actions 
     */
    subscribe(eventName, actions) {
        if (!(eventName in this._subscriptions)) {
            this._subscriptions[eventName] = [];
        }

        const subscription = new Subscription(actions);

        this._subscriptions[eventName].push(subscription);
    }

    async _load() {
        // Load all plugins
        for (const plugin of this._plugins) {
            await Promise.resolve(plugin.load(this));
        }

        this._session = new lt.session({
            alert_mask: lt.alert.all_categories,
            peer_fingerprint: lt.generate_fingerprint('PO', 0, 0, 1),
            user_agent: 'Porla/alpha libtorrent/1.2.0'
        });

        await this.emit('session.init', this._session);

        try {
            const stateFile = path.join(this._options.dataPath, 'state.dat');
            const buffer = await fs.readFile(stateFile);
            this._session.load_state(buffer);
        } catch (err) {
            // We accept the ENOENT error since the data folder or state.dat
            // file might not exist yet.
            if (err.code !== 'ENOENT') {
                this.log.error(
                    err,
                    'Unknown error when loading state',
                    { file: stateFile });
            }
        }

        // Load all torrent state
        try {
            const files = await fs.readdir(this._options.dataPath);
            const dats = files.filter(x => path.extname(x) === '.json');
            const items = [];

            for (const dat of dats) {
                const p = path.join(this._options.dataPath, dat);
                const buf = await fs.readFile(p);
                const decoded = JSON.parse(buf);

                if (decoded !== Object(decoded)) {
                    this.log.warn('.dat file not an object', { file: p });
                    // TODO: err for dat file. should log warn and remove
                    // dat file and corresponding torrent file (if it exists)
                    continue;
                }

                const resumeBuffer = Buffer.from(decoded.resume, 'base64');
                let queuePosition = Number.MAX_VALUE;

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
                item.params.ti = new lt.torrent_info(tf);

                this._mutedTorrents.push(item.infoHash);
                this._session.async_add_torrent(item.params);
            }
        } catch (err) {
            // We accept the ENOENT error since the data folder might not exist
            // yet.
            if (err.code !== 'ENOENT') {
                this.log.error(err, 'Unknown error when loading torrents');
            }
        }

        this._readAlerts();

        this._timers.postUpdates = setInterval(this._postUpdates.bind(this), 1000);
        this._timers.readMonitors = setTimeout(this._readMonitors.bind(this));
    }

    _postUpdates() {
        this._session.post_dht_stats();
        this._session.post_session_stats();
        this._session.post_torrent_updates();
    }

    async _readAlerts() {
        do {
            const foundAlerts = await this._waitForAlerts(100);

            if (!foundAlerts) {
                continue;
            }

            const alerts = this._session.pop_alerts();

            for (const alert of alerts) {
                switch (alert.what) {
                    case 'add_torrent':
                        // TODO: check error

                        const status = alert.handle.status();

                        // Check if this torrent is a muted torrent - i.e it
                        // was added as a part of the load function.
                        if (this._mutedTorrents.includes(status.info_hash)) {
                            const idx = this._mutedTorrents.indexOf(status.info_hash);
                            this._mutedTorrents.splice(idx, 1);
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

                        this.emit(
                            'torrent.added',
                            { torrent: this._torrents.get(status.info_hash) });

                        break;

                    case 'session_stats':
                        const metrics = Object.keys(this._sessionMetrics);
                        const sessionStats = metrics.reduce((acc, curr, idx) => {
                            const metric = metrics[idx];
                            acc[metric] = alert.counters[this._sessionMetrics[metric].index];
                            return acc;
                        }, {});

                        this.emit('session.statistics', { stats: sessionStats });

                        // TODO: Map the stats to a usable object. We want to
                        // support graphs for gauges, so store these in an object
                        // based on user specified intervals - i.e the user can
                        // specify the following object,
                        // {
                        //     'gauge-1': [ '1m/1', '5m/10', '60m/60' ]
                        // }
                        // This will store values for 'gauge-1' at a sample rate
                        // of 1 second for the 1 minute graph, 10 seconds for the
                        // 5 minute graph, and 60 seconds for the 60 minute graph.
                        break;

                    case 'state_update':
                        const torrentsStats = alert.status.reduce((acc, curr) => {
                            acc.downloadPayloadRate += curr.download_payload_rate;
                            acc.uploadPayloadRate += curr.upload_payload_rate;
                            return acc;
                        }, {
                            downloadPayloadRate: 0,
                            uploadPayloadRate: 0
                        });

                        this.emit('torrents.statistics', { stats: torrentsStats });

                        const updatedTorrents = [];

                        for (const status of alert.status) {
                            const updatedTorrent = this._torrents.get(status.info_hash);
                            updatedTorrent._status = status;

                            updatedTorrents.push(updatedTorrent);
                        }

                        this.emit('torrents.updated', { torrents: updatedTorrents });
                        break;

                    case 'torrent_finished':
                        const finishedTorrent = this._torrents.get(alert.handle.info_hash());
                        this.emit('torrent.finished', { torrent: finishedTorrent });
                        break;

                    case 'torrent_paused':
                        const pausedTorrent = this._torrents.get(alert.handle.info_hash());
                        pausedTorrent.emit('paused');
                        this.emit('torrent.paused', { torrent: pausedTorrent });
                        break;

                    case 'torrent_removed':
                        const removedTorrent = this._torrents.get(alert.info_hash);
                        removedTorrent.emit('removed');
                        this._torrents.delete(alert.info_hash);

                        this.emit('torrent.removed', { infoHash: alert.info_hash });
                        break;
                }
            }
        } while (!this._exiting);

        this._unload();
    }

    async _readMonitors() {
        let promises = [];

        for (const monitor of this._monitors) {
            const promise = Promise.resolve(monitor.getFiles());

            promises.push(promise);
        }

        const result = await Promise.all(promises);

        for (const res of result) {
            for (const buffer of res) {
                const ti = new lt.torrent_info(buffer);

                if (this._torrents.has(ti.info_hash())) {
                    // Torrent already in session.
                    continue;
                }

                const params = new lt.add_torrent_params();
                params.save_path = this._options.savePath;
                params.ti = ti;

                this._session.async_add_torrent(params);
            }
        }

        this._timers.readMonitors = setTimeout(this._readMonitors.bind(this), 5000);
    }

    async _unload() {
        clearInterval(this._timers.postUpdates);
        clearTimeout(this._timers.readMonitors);

        // Unload all plugins.
        for (const plugin of this._plugins) {
            await Promise.resolve(plugin.unload(this));
        }

        const state = this._session.save_state_buf();

        try {
            await fs.mkdir(this._options.dataPath);
        } catch (err) { }

        try {
            await fs.writeFile(path.join(this._options.dataPath, 'state.dat'), state);
        } catch (err) {
            this.log.error(err, 'Failed to save session state');
        }

        this._session.pause();

        let numOutstandingResumeData = 0;

        const tempStatus = this._session.get_torrent_status();

        for (const st of tempStatus) {
            if (!st.handle.is_valid() || !st.has_metadata || !st.need_save_resume) {
                continue;
            }

            st.handle.save_resume_data();
            numOutstandingResumeData += 1;
        }

        while (numOutstandingResumeData > 0) {
            const foundAlerts = await this._waitForAlerts(1000);
            if (!foundAlerts) { continue; }

            const alerts = this._session.pop_alerts();

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

        process.exit();
    }

    async _waitForAlerts(timeout) {
        return new Promise((resolve, reject) => {
            this._session.wait_for_alert(timeout, (err, result) => {
                if (err) {
                    return reject(err);
                }

                return resolve(result);
            });
        });
    }
}

/**
 * Check if a URL has a protocol.
 * @param {string} url
 */
function hasProtocol(url) {
    return /[a-z|A-Z|]+[:]/g.test(url);
}

module.exports = Porla;
