const logs = [];

function logger() {
    if (arguments.length === 0) {
        return Array.from(logs);
    }

    logger.info.apply(logger, arguments);
};

logger.outputs = [];

logger.error = function error(err, template, parameters) {
}

logger.info = function info(template, parameters) {
    logs.push(template);

    for (const output of logger.outputs) {
        output(template);
    }
};

logger.warn = function warn(template, parameters) {
};

module.exports = logger;
