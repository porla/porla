const logger = require('./logger');
const Subscription = require('./subscription');

class AsyncEventEmitter {
    constructor() {
        this.subscriptionGroups = new Map();
    }

    async emit(eventName, ...args) {
        if (!(this.subscriptionGroups.has(eventName))) {
            return Promise.resolve();
        }
    
        const subscriptions = this.subscriptionGroups.get(eventName);
    
        for (const subscription of subscriptions) {
            try {
                await subscription.execute(...args);
            } catch (err) {
                logger.error(`Error when executing subscription group for ${eventName}: ${err}`);
            }
        }
    }

    on(eventName, handlerGroup) {
        if (!this.subscriptionGroups.has(eventName)) {
            this.subscriptionGroups.set(eventName, []);
        }
    
        const subs = this.subscriptionGroups.get(eventName);
        subs.push(new Subscription(handlerGroup));
    
        this.subscriptionGroups.set(eventName, subs);
    };
}


module.exports = AsyncEventEmitter;
