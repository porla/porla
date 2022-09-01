import lt from "@porla/libtorrent";
import type { Database } from "better-sqlite3";
import EventEmitter from "events";
import { logger } from "./logger.js";
import AddTorrentParams from "./models/addtorrentparams.js";

interface AddParams {
  magnet_link: string;
  save_path: string;
}

interface ITorrent {
  get download_payload_rate(): number;
  get name(): string;
  get num_peers(): number;
  get save_path(): string;
  get size(): number;
  get state(): number;
  get upload_payload_rate(): number;
}

export interface ISession {
  add(params: AddParams): void;
  reloadSettings(): void;
  torrents(): ITorrent[];
}

export default class Session extends EventEmitter implements ISession {
  readonly #db: Database;
  readonly #session: lt.Session;
  readonly #timer: NodeJS.Timer;
  readonly #torrents: Map<string, lt.TorrentStatus>;

  constructor(db: Database) {
    super();

    this.#db = db;
    this.#session = new lt.Session();
    this.#session.on("add_torrent", _ => this.#onAddTorrent(_));
    this.#session.on("metadata_received", _ => this.#onMetadataReceived(_));
    this.#session.on("save_resume_data", _ => this.#onSaveResumeData(_));
    this.#session.on("state_update", _ => this.#onStateUpdate(_));
    this.#torrents = new Map<string, lt.TorrentStatus>();

    this.#timer = setInterval(_ => this.#postUpdates(), 1000);
  }

  add(params: AddParams) {
    const p = lt.parse_magnet_uri(params.magnet_link);
    p.save_path = params.save_path;
    this.#session.add_torrent(p);
  }

  async load() {
    const params = AddTorrentParams.all(this.#db);

    logger.info("Loading %d torrent(s)", params.length);

    for (const param of params) {
      this.#session.add_torrent(param);
    }
  }

  reloadSettings(): void {
    const config = this.#db
      .prepare(`SELECT key,value
                FROM config`)
      .all()
      .reduce((prev, current) => {
        return {
          ...prev,
          [current.key]: JSON.parse(current.value)
        }
      }, {});

    const settings = lt.default_settings();

    for (const key of Object.keys(config)) {
      const val = config[key];
      switch (key) {
        case "proxy_type": settings.proxy_type = val; break;
        case "proxy_hostname": settings.proxy_hostname = val; break;
        case "proxy_port": settings.proxy_port = val; break;
      }
    }

    this.#session.apply_settings(settings);
  }

  torrents(): ITorrent[] {
    const torrents: ITorrent[] = [];

    for (const [,value] of this.#torrents) {
      torrents.push({
        download_payload_rate: value.download_payload_rate,
        name: value.name,
        num_peers: 0,
        save_path: value.save_path,
        size: value.total_wanted,
        state: value.state,
        upload_payload_rate: value.upload_payload_rate
      });
    }

    return torrents;
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

    for (const [_, torrent] of this.#torrents) {
      if (torrent.need_save_resume) {
        outstanding++;

        torrent.handle.save_resume_data(
          lt.resume_data_flags_t.flush_disk_cache
            | lt.resume_data_flags_t.save_info_dict
            | lt.resume_data_flags_t.only_if_modified);
      }
    }

    if (outstanding <= 0) {
      // No torrents needs state saved.
      return;
    }

    logger.info("Saving resume data for %d torrent(s)", outstanding);

    return new Promise<void>(resolve => {
      this.#session.on("save_resume_data", data => {
        AddTorrentParams.update(
          this.#db,
          data.params,
          data.handle.queue_position());

        logger.info("Resume data saved for %s", data.torrent_name);

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
    if (d.error) {
      logger.error(
        "Failed to add torrent %s: %s",
        d.torrent_name,
        d.error.message);

      return;
    }

    // TODO: when we support user_data on lt.AddTorrentParams we
    //       can probably solve this in a better way.

    if (!AddTorrentParams.exists(this.#db, d.handle.info_hashes())) {
      AddTorrentParams.insert(this.#db, d.params, d.handle.queue_position());
      logger.info("Torrent %s added", d.torrent_name);
    }

    const hash = d.handle.info_hashes();

    this.#torrents.set(
      hash.v1 || "",
      d.handle.status());
  }

  #onMetadataReceived(d: lt.MetadataReceivedAlert) {
    logger.info(
      "Metadata received for %s - saving resume data",
      d.torrent_name);

    d.handle.save_resume_data(
      lt.resume_data_flags_t.flush_disk_cache
        | lt.resume_data_flags_t.save_info_dict
        | lt.resume_data_flags_t.only_if_modified);
  }

  #onSaveResumeData(d: lt.SaveResumeDataAlert) {
    AddTorrentParams.update(
      this.#db,
      d.params,
      d.handle.queue_position());
  }

  #onStateUpdate(d: lt.StateUpdateAlert) {
    for (const status of d.status) {
      this.#torrents.set(
        status.info_hashes.v1 || "",
        status);
    }
  }

  #postUpdates() {
    this.#session.post_dht_stats();
    this.#session.post_session_stats();
    this.#session.post_torrent_updates();
  }
}
