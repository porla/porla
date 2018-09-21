const lt = require('@porla/libtorrent');
const Plugin = require('./lib/plugin');
const Porla = require('./lib/porla');

module.exports = {
    // Re-export @porla/libtorrent in case we need it from plugins. Ideally,
    // this should not be needed as Porla should provide the APIs - but you
    // never know ¯\_(ツ)_/¯
    lt: lt,

    Plugin: Plugin,
    Porla: Porla
};
