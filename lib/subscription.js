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

module.exports = Subscription;
