import Database from "better-sqlite3";
import express from "express";
import * as fs from "fs/promises";
import path from "path";
import * as trpc from '@trpc/server';
import * as trpcExpress from '@trpc/server/adapters/express';
import { appRouter } from "./router";

// created for each request
const createContext = ({
  req,
  res,
}: trpcExpress.CreateExpressContextOptions) => ({}) // no context

type Context = trpc.inferAsyncReturnType<typeof createContext>;

const app = express();
app.use(
  '/trpc',
  trpcExpress.createExpressMiddleware({
    router: appRouter,
    createContext,
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
