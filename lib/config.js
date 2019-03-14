const fs = require('fs');
const logger = require('./logger');
const path = require('path');
const promisify = require('util').promisify;
const readFile = promisify(fs.readFile);
const writeFile = promisify(fs.writeFile);

let config = {};

module.exports = {
    async load(options) {
        const configFile = path.join(options.dataPath, 'porla.json');

        if (fs.existsSync(configFile)) {
            const buffer = await readFile(configFile);
            const loadedConfig = JSON.parse(buffer);

            if (loadedConfig !== Object(loadedConfig)) {
                logger.warn('Invalid configuration file - root not an object');
                return;
            }

            config = loadedConfig;

            logger.info(`Loading configuration from ${configFile}`);
        }
    },

    async save(options) {
        const configFile = path.join(options.dataPath, 'porla.json');

        try {
            await writeFile(configFile, JSON.stringify(config, null, 2));
            logger.info(`Configuration saved to ${configFile}`);
        } catch (err) {
            logger.error(`Failed to save configuration: ${err}`);
        }
    },

    async get(key) {
        return config[key];
    },

    async set(key, value) {
        config[key] = value;
    }
};
