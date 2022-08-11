import Database, { Database as DatabaseType } from "better-sqlite3";
import express from "express";
import * as trpcExpress from '@trpc/server/adapters/express';
import { appRouter } from "./router";
import migrate from "./migrate";
import { logger } from "./logger";
import Session, { ISession } from "./session";

export interface IPlugin {
}

export interface IPluginLoader {
  load(): Promise<IPlugin>;
}

export interface IHost {
  database(): DatabaseType;
  session(): ISession;
}

async function main() {
  const db = new Database(":memory:");

  try {
    await migrate(db);
  } catch (err) {
    logger.fatal(err, "Failed to apply migrations");
    return;
  }

  const s = new Session(db);

  try {
    await s.load();
  } catch (err) {
    logger.fatal(err, "Failed to load session");
    return;
  }

  const app = express();
  app.use(
    '/trpc',
    trpcExpress.createExpressMiddleware({
      router: appRouter,
      createContext: () => {
        return {
          session: () => s
        }
      },
    }));

  app.listen(4999);
}

main()
  .catch(err => logger.fatal(err, "Porla experienced an error. Shutting down..."));
