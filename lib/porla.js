
class Subscription {
    constructor(actions) {
        this.actions = actions;
    }

    async execute(parameter) {
        for (const action of this.actions) {
            const result = await Promise.resolve(action(parameter));

            if (typeof result === 'boolean' && !result) {
                break;
            }
        }
    }
}

class Porla {
    constructor(options) {
        options = options || {};
        this.subscriptions = {};
    }

    async emit(eventName, parameter) {
        if (!(eventName in this.subscriptions)) {
            return Promise.resolve();
        }

        for (const subscription of this.subscriptions[eventName]) {
            await subscription.execute(parameter);
        }
    }

    subscribe(eventName, actions) {
        if (!(eventName in this.subscriptions)) {
            this.subscriptions[eventName] = [];
        }

        const subscription = new Subscription(actions);

        this.subscriptions[eventName].push(subscription);
    }
}

module.exports = Porla;
