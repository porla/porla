class Subscription {
    constructor(actions) {
        if (!Array.isArray(actions)) {
            actions = [ actions ];
        }

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

module.exports = Subscription;
