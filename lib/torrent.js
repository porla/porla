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

    get progress() {
        return this._status.progress;
    }

    get savePath() {
        return this._status.save_path;
    }

    get stats() {
        return {
            size: this._status.total,
            sizeWanted: this._status.total_wanted
        };
    }

    files() {
        const ti = this._status.torrent_file;

        if (!ti) {
            return [];
        }

        const f = [];

        const fls = ti.files();
        const prg = this._handle.file_progress();

        for (let i = 0; i < ti.num_files(); i++) {
            let fileProgress = 0;

            if (prg[i] > 0) {
                fileProgress = prg[i] / fls.file_size(i);
            }

            f.push({
                name: fls.file_name(i),
                path: fls.file_path(i),
                progress: fileProgress,
                size: fls.file_size(i),
                sizeDone: prg[i]
            });
        }

        return f;
    }

    pause() {
        this._handle.pause();

        return new Promise((resolve) => {
            this.once('paused', () => resolve());
        });
    }
}

module.exports = Torrent;
