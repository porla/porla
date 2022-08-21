import { Database } from "better-sqlite3";
import * as fs from "fs/promises";
import path from "path";
import { logger } from "./logger.js";

export interface IMigrationOptions {
  db: Database;
  files: Array<string>;
  key: string;
}

export async function migrate(opts: IMigrationOptions) {
  logger.info(
    "Running %d database migrations for key '%s'",
    opts.files.length,
    opts.key);

  const {
    db,
    files,
    key
  } = opts;

  try {
    db.exec(`CREATE TABLE IF NOT EXISTS migrations (
      id INTEGER PRIMARY KEY,
      key TEXT NOT NULL,
      migration TEXT NOT NULL,
      applied_utc INTEGER NOT NULL,
      UNIQUE (key,migration)
    );`);
  } catch (err) {
    logger.fatal(err, "Failed to create migrations table");
    throw err;
  }

  for (const file of files.sort()) {
    const buf = await fs.readFile(file, "utf-8");
    const name = path.basename(file);

    const { cnt } = db.prepare("SELECT COUNT(*) AS cnt FROM migrations WHERE key=? AND migration=?")
      .get(key, name);

    if (cnt > 0) {
      continue;
    }

    db.exec(buf);
    db.prepare("INSERT INTO migrations (key,migration,applied_utc) VALUES (?,?,?);")
      .run(key, name, new Date().getUTCMilliseconds());

    logger.info("Migration %s applied", name);
  }
}
