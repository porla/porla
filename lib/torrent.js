const { EventEmitter } = require('events');
const lt = require('@porla/libtorrent');

class Torrent extends EventEmitter {
    /**
     * Creates a new Torrent instance.
     * @param {lt.torrent_handle} handle 
     */
    constructor(handle) {
        super();

        this._handle   = handle;
        this._metadata = {};
        this._status   = handle.status();
    }

    get infoHash() {
        return this._status.info_hash;
    }

    get metadata() {
        return this._metadata;
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

    get state() {
        return this._status.state;
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

    move(path) {
        this._handle.move_storage(path);

        return new Promise((resolve, reject) => {
            function moved() {
                this.off('storage_moved_failed', moveFailed);
                resolve();
            }

            function moveFailed(err) {
                this.off('storage_moved', moved);
                reject(err);
            }

            this.once('storage_moved',        moved);
            this.once('storage_moved_failed', moveFailed);
        });
    }

    pause() {
        this._handle.pause();

        return new Promise((resolve) => {
            this.once('paused',() => resolve());
        });
    }
}

module.exports = Torrent;
