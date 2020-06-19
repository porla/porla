const config = require('./config');
const logger = require('./logger');
const path = require('path');
const process = require('process');
const session = require('./session');

const callbacks = {};

async function emit(eventName, argument) {
    if (!(eventName in callbacks)) {
        return;
    }

    const prms = callbacks[eventName]
        .map(callback => new Promise(async (resolve, reject) => {
            let result;

            try {
                result = await Promise.resolve(callback(argument));
            } catch (err) {
                reject(err);
                return;
            }

            if (Array.isArray(result)) {
                for (const item of result) {
                    let itemResult;

                    try {
                        itemResult = await Promise.resolve(item(argument));
                    } catch (err) {
                        logger.error('Failed to execute callback item: %s', err);
                        reject(err);
                        return;
                    }

                    if (typeof itemResult === 'boolean' && itemResult === false) {
                        break;
                    }
                }
            }

            resolve();
        }));

    return Promise.all(prms);
}

function registerSubscription(eventName, callback) {
    if (!(eventName in callbacks)) {
        callbacks[eventName] = [];
    }

    callbacks[eventName].push(callback);
}

async function load(options) {
    // load config
    await config.load(options);

    const plugins = options.plugins || [];

    const pluginHost = {
        addMagnetLink: session.addMagnetLink,
        addTorrent: session.addTorrent,
        config: config,
        log: logger,
        on: registerSubscription,
        removeTorrent: session.removeTorrent,
        torrents() {
            return session.torrents();
        }
    };

    for (const plugin of plugins) {
        await Promise.resolve(plugin(pluginHost));
    }

    // Set up session events
    const sessionEvents = [
        'session.statistics',
        'torrent.added',
        'torrent.finished',
        'torrent.paused',
        'torrent.removed',
        'torrents.statistics',
        'torrents.updated' ];

    for (const evnt of sessionEvents) {
        session.on(evnt, function emitSessionEvent(arg) {
            const eventArgs = {
                log: logger,
                ...arg
            };

            emit(evnt, eventArgs)
                .catch(err => {
                    logger.error('Failed to emit event: %s', err);
                });
        });
    }

    // Load session
    await session.init(options);
    await emit('session.init', { session });
    await session.load(options);

    // Last event to emit. Most plugins will do their thing here.
    await emit('loaded');
}

function porla(options) {
    options = options || {};

    // Set default options
    options.dataPath = options.dataPath || path.join(process.cwd(), 'data');
    options.savePath = options.savePath || process.cwd();

    process.nextTick(() => load(options));

    const shutdown = async () => {
        // Emit the shutdown event. This is where plugins should save state
        // and other things they need to do before shutting down. No more
        // events will be emitted after this.
        await emit('shutdown');

        await session.unload();
        await config.save(options);

        logger.info('Porla has been shut down. Bye.');
    }

    // Install the SIGINT handler if we don't explicitly disable it
    let installSignalHandler = true;

    if ('installSignalHandler' in options) {
        installSignalHandler = !!(options.installSignalHandler);
    }

    if (installSignalHandler) {
        logger.info('Installing SIGINT handler');

        process.on('SIGINT', async () => {
            await shutdown();
            process.exit(0);
        });
    }

    return {
        on: registerSubscription,
        shutdown
    }
}

porla.SOCKS5 = 2;

module.exports = porla;
