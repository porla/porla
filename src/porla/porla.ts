import Database from "better-sqlite3";
import express from "express";
import * as fs from "fs/promises";
import path from "path";
import * as lt from "@porla/libtorrent";
import * as trpcExpress from '@trpc/server/adapters/express';
import { appRouter } from "./router";

const s = new lt.Session();

const app = express();
app.use(
  '/trpc',
  trpcExpress.createExpressMiddleware({
    router: appRouter,
    createContext: ({ req, res }) => {
      return {
        session: () => s
      }
    },
  }));

const db = new Database(":memory:");

async function migrate() {
  const sqlDir = path.join(__dirname, "sql");
  const migrations = await fs.readdir(sqlDir);

  for (const migration of migrations) {
    const buf = await fs.readFile(path.join(sqlDir, migration));
    db.exec(buf.toString("utf-8"));
  }
}

async function loadTorrents() {
}

Promise.resolve()
  .then(migrate)
  .then(loadTorrents)
  .then(() => {
    app.listen(4999);
  });
