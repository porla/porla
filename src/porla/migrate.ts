import { Database } from "better-sqlite3";
import * as fs from "fs/promises";
import path from "path";
import { fileURLToPath } from "url";
import { logger } from "./logger.js";

export default async function migrate(db: Database) {
  const __dirname = path.dirname(
    fileURLToPath(import.meta.url));

  const sqlDir = path.join(__dirname, "sql");
  const migrations = await fs.readdir(sqlDir);

  logger.info("Preparing to run database migrations");

  const {user_version} = db.prepare("PRAGMA user_version;").get();

  if (user_version === migrations.length) {
    logger.info("Database up to date");
    return;
  }

  logger.info("Migrating from version %d to version %d", user_version, migrations.length);

  for (let i = user_version; i < migrations.length; i++) {
    const migration = migrations[i];
    const buf = await fs.readFile(path.join(sqlDir, migration));

    logger.info("Applying migration %s (%d byte(s))", migration, buf.length);

    db.exec(buf.toString("utf-8"));
  }

  logger.info("Setting user version");

  db.exec(`PRAGMA user_version=${migrations.length}`);
}
