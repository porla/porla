import * as trpc from "@trpc/server";
import { Database } from "better-sqlite3";
import { z } from "zod";
import { ISession } from "./session.js";

type PorlaContext = trpc.inferAsyncReturnType<() => {
  database(): Database;
  session(): ISession
}>;

export const appRouter = trpc
  .router<PorlaContext>()
  .query("config.get", {
    input: z.string().array(),
    resolve(req) {
      const qs = req.input
        .map(_ => "?")
        .join(",");

      return req.ctx.database()
        .prepare(`SELECT key,value FROM config WHERE key IN (${qs})`)
        .all(req.input)
        .reduce((prev, current) => {
          return {
            ...prev,
            [current["key"]]: JSON.parse(current.value)
          }
        }, {});
    }
  })
  .query("torrents.list", {
    resolve(req) {
      return req.ctx.session()
        .torrents()
        .map(t => {
          return {
            download_rate: t.download_payload_rate,
            flags: t.flags,
            info_hash: t.info_hash,
            name: t.name,
            num_peers: t.num_peers,
            num_seeds: t.num_seeds,
            progress: t.progress,
            save_path: t.save_path,
            size: t.size,
            state: t.state,
            upload_rate: t.upload_payload_rate
          }
        });
    },
  })
  .mutation("config.set", {
    input: z.object({
      default_save_path: z.string().optional(),
      proxy_type: z.number().optional(),
      proxy_hostname: z.string().optional(),
      proxy_port: z.number().optional()
    }),
    resolve(req) {
      const insert = req.ctx.database()
        .prepare(`REPLACE INTO config(key,value) VALUES (?,?);`);

      const tx = req.ctx.database()
        .transaction(vals => {
          for (const val of vals) {
            insert.run(val);
          }
        });

      const v = Object.keys(req.input)
        .map(key => {
          return [ key, JSON.stringify((req.input as any)[key]) ];
        });

      tx(v);

      req.ctx.session()
        .reloadSettings();
    }
  })
  .mutation("torrents.add", {
    input: z.object({
      magnet_link: z.string(),
      save_path: z.string()
    }),
    resolve(req) {
      req.ctx.session()
        .add(req.input);
    },
  })
  .mutation("torrents.pause", {
    input: z.array(z.string().nullable()).length(2),
    resolve(req) {
      req.ctx.session()
        .pause([ req.input[0], req.input[1]]);
    }
  })
  .mutation("torrents.resume", {
    input: z.array(z.string().nullable()).length(2),
    resolve(req) {
      req.ctx.session()
        .resume([ req.input[0], req.input[1]]);
    }
  });

export type AppRouter = typeof appRouter;
