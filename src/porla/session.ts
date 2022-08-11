import lt from "@porla/libtorrent";
import { Database } from "better-sqlite3";
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

  #onAddTorrent(d: lt.AddTorrentAlert) {
  }
}
