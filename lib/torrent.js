const { EventEmitter } = require('events');
const lt = require('@porla/libtorrent');

class Torrent extends EventEmitter {
    /**
     * Creates a new Torrent instance.
     * @param {lt.torrent_handle} handle 
     */
    constructor(handle) {
        super();

        this._handle = handle;
        this._status = handle.status();
    }

    get infoHash() {
        return this._status.info_hash;
    }

    get name() {
        return this._status.name;
    }

    pause() {
        this._handle.pause();

        return new Promise((resolve) => {
            this.once('paused',() => resolve());
        });
    }
}

module.exports = Torrent;
