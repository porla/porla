const config = require('./config');
const logger = require('./logger');
const path = require('path');
const process = require('process');
const session = require('./session');
const Subscription = require('./subscription');

const subscriptionGroups = new Map();

const emit = async (eventName, argument) => {
    if (!(subscriptionGroups.has(eventName))) {
        return Promise.resolve();
    }

    const subscriptions = subscriptionGroups.get(eventName);

    for (const subscription of subscriptions) {
        try {
            await subscription.execute(argument);
        } catch (err) {
            logger.error('Error when executing subscription group for %s: %s', eventName, err);
        }
    }
};

const registerSubscription = (eventName, handlerGroup) => {
    if (!subscriptionGroups.has(eventName)) {
        subscriptionGroups.set(eventName, []);
    }

    const subs = subscriptionGroups.get(eventName);
    subs.push(new Subscription(handlerGroup));

    subscriptionGroups.set(eventName, subs);
};

const load = async (options) => {
    // load config
    await config.load(options);

    const plugins = options.plugins || [];

    const pluginHost = {
        addMagnetLink: session.addMagnetLink,
        addTorrent: session.addTorrent,
        config: config,
        log: logger,
        on: registerSubscription
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
        session.on(evnt, async (arg) => {
            await emit(evnt, arg);
        });
    }

    // Load session
    await session.init(options);
    await emit('session.init', { session });
    await session.load(options);

    // Last event to emit. Most plugins will do their thing here.
    await emit('loaded');
}

const initPorla = (options) => {
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

initPorla.SOCKS5 = 2;

module.exports = initPorla;
