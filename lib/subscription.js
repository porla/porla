class Subscription {
    constructor(actions) {
        if (!Array.isArray(actions)) {
            actions = [ actions ];
        }

        this.actions = actions;
    }

    async execute(...args) {
        for (const action of this.actions) {
            const result = await Promise.resolve(action(...args));

            if (typeof result === 'boolean' && !result) {
                break;
            }
        }
    }
}

module.exports = Subscription;
