#!/usr/bin/env node

import Database from "better-sqlite3";
import * as path from "path";
import { logger } from "./logger.js";
import migrate from "./migrate.js";
import { IHost } from "./porla.js";
import Session from "./session.js";

type PluginLoader = (host: IHost) => Promise<void>;

interface IPorlaConfigJs {
  db?: string;
  plugins?: Array<PluginLoader>
}

async function main() {
  logger.info("Porla");

  let config: IPorlaConfigJs = {};

  try {
    const configFile = path.join(process.cwd(), "porla.config.js");
    config = await import(configFile) as IPorlaConfigJs;
  } catch (err) {}

  for (const loader of config?.plugins || []) {
  }

  const db = new Database(
    config?.db
      || path.join(process.cwd(), "porla.db"));

  try {
    await migrate(db);
  } catch (err) {
    logger.fatal(err, "Failed to apply migrations");
    return process.exit(1);
  }

  const s = new Session(db);

  try {
    await s.load();
  } catch (err) {
    logger.fatal(err, "Failed to load session");
    return;
  }
}

main().catch(console.error);
