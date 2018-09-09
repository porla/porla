
class Torrent {
    /**
     * Creates a new Torrent instance.
     * @param {lt.torrent_handle} handle 
     */
    constructor(handle) {
        this._handle = handle;
        this._status = handle.status();
    }

    get name() {
        return this._status.name;
    }
}

module.exports = Torrent;
