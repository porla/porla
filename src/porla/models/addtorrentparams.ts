import { Database } from "better-sqlite3";
import lt from "@porla/libtorrent";

export default class AddTorrentParams {
  static all(db: Database): lt.AddTorrentParams[] {
    return db.prepare(`SELECT resume_data_buf
                       FROM addtorrentparams
                       ORDER BY queue_position ASC`)
      .all()
      .map(r => lt.read_resume_data(r.resume_data_buf));
  }

  static exists(db: Database, hash: lt.InfoHash): boolean {
    return db.prepare(
      `SELECT COUNT(*) as cnt
       FROM addtorrentparams
       WHERE (info_hash_v1 = $v1 AND info_hash_v2 IS NULL)
         OR (info_hash_v1 IS NULL AND info_hash_v2 = $v2)
         OR (info_hash_v1 = $v1 AND info_hash_v2 = $v2);`)
      .get({
        v1: hash.has_v1() ? hash.v1 : null,
        v2: hash.has_v2() ? hash.v2 : null
      }).cnt > 0;
  }

  static insert(db: Database, params: lt.AddTorrentParams, queuePosition: number) {
    const buf = lt.write_resume_data_buf(params);

    db.prepare(
      `INSERT INTO addtorrentparams(info_hash_v1, info_hash_v2, queue_position, resume_data_buf)
       VALUES (?,?,?,?);`)
      .run([
        params.info_hashes.has_v1() ? params.info_hashes.v1 : null,
        params.info_hashes.has_v2() ? params.info_hashes.v2 : null,
        queuePosition,
        buf
      ]);
  }

  static update(db: Database, params: lt.AddTorrentParams, queuePosition: number) {
    const buf = lt.write_resume_data_buf(params);

    db.prepare(
      `UPDATE addtorrentparams
        SET queue_position = $qp,
            resume_data_buf = $rdf
        WHERE (info_hash_v1 = $v1 AND info_hash_v2 IS NULL)
          OR (info_hash_v1 IS NULL AND info_hash_v2 = $v2)
          OR (info_hash_v1 = $v1 AND info_hash_v2 = $v2);`)
      .run({
        qp: queuePosition,
        rdf: buf,
        v1: params.info_hashes.has_v1() ? params.info_hashes.v1 : null,
        v2: params.info_hashes.has_v2() ? params.info_hashes.v2 : null
      });
  }
}
