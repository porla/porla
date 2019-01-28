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
            logger.error(`Error when executing subscription group for ${eventName}: ${err}`);
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
    const plugins = options.plugins || [];

    const pluginHost = {
        addTorrent: session.addTorrent,
        config: {
            get(key) {
            },
            set(key, value) {
            }
        },
        log: logger,
        on: registerSubscription
    };

    for (const plugin of plugins) {
        await Promise.resolve(plugin(pluginHost));
    }

    // Set up session events
    session.on('torrent.added', async (torrent) => {
        await emit('torrent.added', { torrent: torrent });
    });

    // Load session
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

    return {
        on: registerSubscription,
        shutdown: async () => {
            // Emit the shutdown event. This is where plugins should save state
            // and other things they need to do before shutting down. No more
            // events will be emitted after this.
            await emit('shutdown');

            await session.unload();

            logger.info('Porla has been shut down. Bye.');
        }
    }
}

module.exports = initPorla;
