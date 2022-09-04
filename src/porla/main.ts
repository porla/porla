#!/usr/bin/env node

import { createExpressMiddleware } from "@trpc/server/adapters/express/dist/trpc-server-adapters-express.cjs.js";
import Database from "better-sqlite3";
import express from "express";
import * as fs from "fs/promises";
import http from "http";
import * as path from "path";
import { fileURLToPath } from "url";
import { logger } from "./logger.js";
import { migrate } from "./migrator.js";
import { PluginLoader } from "./porla.js";
import { appRouter } from "./router.js";
import Session from "./session.js";

const __dirname = path.dirname(
  fileURLToPath(import.meta.url));

const HTTP_PORT_DEFAULT = 4999;

interface IPorlaConfigJs {
  db?: string;
  plugins?: Array<PluginLoader>
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
    const files = await fs.readdir(path.join(__dirname, "sql"))
      .then(f => f.map(file => path.join(__dirname, "sql", file)));

    await migrate({ db, files, key: "porla" });
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
          database: () => db,
          session: () => s
        }
      },
    }));

  const unloaders = new Array<() => Promise<void>>();

  for (const loader of config?.plugins || []) {
    try {
      await loader({
        database: () => db,
        express: () => app,
        onUnload: (handler) => unloaders.push(handler),
        session: () => s,
      });
    } catch (err) {
      logger.error(err, "Failed to load plugin (%s).", loader.name);
    }
  }

  const httpServer = http.createServer(app);
  httpServer.listen(HTTP_PORT_DEFAULT, () => {
    logger.info(`All done. Visit http://localhost:${HTTP_PORT_DEFAULT} to access Porla`);
  });

  let shuttingDown = false;

  async function exit() {
    if (shuttingDown) { return; }
    shuttingDown = true;

    logger.info("Interrupt received - shutting down");

    let exitCode = 0;

    try {
      try {
        await Promise.all(unloaders);
      } catch (err) {
        logger.error(err, "Error when unloading plugins");
      }

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
