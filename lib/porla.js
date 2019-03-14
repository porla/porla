const config = require('./config');
const AsyncEventEmitter = require('./asyncEventEmitter');
const logger = require('./logger');
const Mustache = require('mustache');
const path = require('path');
const process = require('process');
const proxyType = require('./proxyType');
const Session = require('./session');

class PorlaInstance extends AsyncEventEmitter {
    constructor(options) {
        super();
        this._options = options;
        this._session = new Session(options);
    }

    async load() {
        const plugins = this._options.plugins || [];

        const pluginHost = {
            addTorrent: (...args) => {
                return this._session.addTorrent(...args);
            },

            config: {
                get: config.get,
                set: config.set
            },
            log: logger,
            on: (...args) => this.on(...args)
        };

        const eventHost = {
            utils: {
                renderMessage(template, view) {
                    return Mustache.render(template, view);
                }
            }
        };

        // Load config the first thing we do
        await Promise.resolve(config.load(this._options));

        // Load each plugin
        for (const plugin of plugins) {
            await Promise.resolve(plugin(pluginHost));
        }

        // Set up session events
        this._session.on('torrent.added', async (torrent) => {
            await this.emit('torrent.added', eventHost, { torrent });
        });

        this._session.on('torrent.finished', async (torrent) => {
            await this.emit('torrent.finished', eventHost, { torrent });
        })

        // Load session
        await this._session.load();

        // Last event to emit. Most plugins will do their thing here.
        await this.emit('loaded');
    }

    async unload() {
        // Emit the shutdown event. This is where plugins should save state
        // and other things they need to do before shutting down. No more
        // events will be emitted after this.
        await this.emit('shutdown');

        await this._session.unload();

        await Promise.resolve(config.save(this._options));

        logger.info('Porla has been shut down. Bye.');
    }
}

const initPorla = (options) => {
    options = options || {};

    // Set default options
    options.dataPath = options.dataPath || path.join(process.cwd(), 'data');
    options.savePath = options.savePath || process.cwd();

    return (function (options) {
        const instance = new PorlaInstance(options);

        process.nextTick(async () => await instance.load());

        return {
            on(...args) {
                instance.on(...args);
            },

            async shutdown() {
                await instance.unload();
            }
        };
    })(options);
}

// Set up other kinds of information on the porla object.

initPorla.SOCKS5 = proxyType.SOCKS5;

module.exports = initPorla;
