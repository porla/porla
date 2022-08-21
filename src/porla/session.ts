import lt from "@porla/libtorrent";
import type { Database } from "better-sqlite3";
import EventEmitter from "events";
import { logger } from "./logger.js";

export interface ISession {
  add(): void;
}

export default class Session extends EventEmitter implements ISession {
  readonly #db: Database;
  readonly #session: lt.Session;
  readonly #torrents: Map<string, lt.TorrentStatus>;

  constructor(db: Database) {
    super();

    this.#db = db;
    this.#session = new lt.Session();
    this.#session.on("add_torrent", _ => this.#onAddTorrent(_));
    this.#torrents = new Map<string, lt.TorrentStatus>();
  }

  add() {
  }

  async load() {
    const params = this.#db.prepare(`SELECT resume_data_buf
                                     FROM addtorrentparams
                                     ORDER BY queue_position ASC`).all();

    logger.info("Loading %d torrents", params.length);
  }

  async unload() {
    // Unload everything in the session and prepare it for deletion
    this.#session.removeAllListeners();

    // Get session state and store it in the database
    const state = lt.write_session_params_buf(
      this.#session.session_state());

    logger.info("Storing %d byte(s) of session state", state.length);

    const {changes} = this.#db.prepare(`INSERT INTO sessionparams (data)
                                        VALUES (?);`).run(state);

    if (changes > 0) {
      logger.info("Session state stored");
    }

    // Remove all sessionparams except the three newest
    this.#db.exec(`DELETE FROM sessionparams
                   WHERE id IN (
                    SELECT id
                    FROM sessionparams
                    ORDER BY timestamp DESC
                    LIMIT -1 OFFSET 3
                   );`);

    this.#session.pause();

    let outstanding = -1;

    this.#torrents.forEach(ts => {
      if (ts.need_save_resume) {
        outstanding++;
      }
    });

    if (outstanding <= 0) {
      // No torrents needs state saved.
      return;
    }

    logger.info("Saving resume data for %d torrent(s)", outstanding);

    return new Promise<void>(resolve => {
      this.#session.on("save_resume_data", data => {
        logger.info("Resume data saved for %s", data.torrent_name);

        const buf = lt.write_resume_data_buf(data.params);
        const pos = data.params.queue_position;

        this.#db.prepare(
          `UPDATE addtorrentparams
           SET queue_position = $1,
               resume_data_buf = $2
           WHERE (info_hash_v1 = $3 AND info_hash_v2 IS NULL)
              OR (info_hash_v1 IS NULL AND info_hash_v2 = $4)
              OR (info_hash_v1 = $3 AND info_hash_v2 = $4);`)
           .run(
            pos,
            buf,
            data.params.info_hashes.has_v1() ? data.params.info_hashes.v1 : null,
            data.params.info_hashes.has_v2() ? data.params.info_hashes.v2 : null);

        outstanding--;

        if (outstanding <= 0) {
          this.#session.removeAllListeners();
          return resolve();
        }
      });

      this.#session.on("save_resume_data_failed", data => {
        logger.warn(
          "Failed to save resume data for %s (%s)",
          data.torrent_name,
          data.error?.message);

        outstanding--;

        if (outstanding <= 0) {
          this.#session.removeAllListeners();
          return resolve();
        }
      });
    })
  }

  #onAddTorrent(d: lt.AddTorrentAlert) {
  }
}
