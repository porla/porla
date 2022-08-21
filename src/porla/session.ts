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

  constructor(db: Database) {
    super();

    this.#db = db;
    this.#session = new lt.Session();
    this.#session.on("add_torrent", _ => this.#onAddTorrent(_));
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
  }

  #onAddTorrent(d: lt.AddTorrentAlert) {
  }
}
