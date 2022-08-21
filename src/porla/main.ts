#!/usr/bin/env node

import { createExpressMiddleware } from "@trpc/server/adapters/express/dist/trpc-server-adapters-express.cjs.js";
import Database, { Database as DbType } from "better-sqlite3";
import express from "express";
import http from "http";
import * as path from "path";
import { fileURLToPath } from "url";
import { logger } from "./logger.js";
import migrate from "./migrate.js";
import { IHost, PluginLoader } from "./porla.js";
import { appRouter } from './router.js';
import Session, { ISession } from "./session.js";

const __dirname = path.dirname(
  fileURLToPath(import.meta.url));

const HTTP_PORT_DEFAULT = 4999;

interface IPorlaConfigJs {
  db?: string;
  plugins?: Array<PluginLoader>
}

class Host implements IHost {
  readonly #app: express.Application;
  readonly #db: Database.Database;

  constructor(app: express.Application, db: DbType, session: ISession) {
    this.#app = app;
    this.#db = db;
  }

  database(): DbType {
    return this.#db;
  }

  express(): express.Application {
    return this.#app;
  }

  session(): ISession {
    throw new Error("Method not implemented.");
  }
}

async function main() {
  logger.info("Porla starting up");

  let config: IPorlaConfigJs = {};

  try {
    const configFile = path.join(process.cwd(), "porla.config.js");
    config = await import(configFile) as IPorlaConfigJs;
  } catch (err) {}

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
    return process.exit(1);
  }

  const app = express();
  app.use(express.static(path.join(__dirname, "html")));
  app.use(
    "/trpc",
    createExpressMiddleware({
      router: appRouter,
      createContext: () => {
        return {
          session: () => s
        }
      },
    }));

  const httpServer = http.createServer(app);
  httpServer.listen(HTTP_PORT_DEFAULT, () => {
    logger.info(`All done. Visit http://localhost:${HTTP_PORT_DEFAULT} to access Porla`);
  });

  for (const loader of config?.plugins || []) {
    await loader(new Host(app, db, s));
  }

  let shuttingDown = false;

  async function exit() {
    if (shuttingDown) { return; }
    shuttingDown = true;

    logger.info("Interrupt received - shutting down");

    let exitCode = 0;

    try {
      await new Promise<void>((resolve, reject) => {
        httpServer.close(err => err ? reject(err) : resolve());
      });

      await s.unload();
    } catch (err) {
      exitCode = 1;
      logger.error(err, "Failed to shutdown properly");
    } finally {
      logger.info("Porla exiting. Bye bye...");
      process.exit(exitCode);
    }
  }

  process.on("SIGINT", exit);
  process.on("SIGQUIT", exit);
  process.on("SIGTERM", exit);

  process.stdin.resume();
}

main().catch(logger.fatal);
