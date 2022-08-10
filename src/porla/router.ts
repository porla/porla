import { Session } from "@porla/libtorrent";
import * as trpc from "@trpc/server";
import { z } from "zod";

type PorlaContext = trpc.inferAsyncReturnType<() => {
  session(): Session
}>;

export const appRouter = trpc
  .router<PorlaContext>()
  .query("torrents.list", {
    async resolve(req) {
      console.log(req.ctx.session());
      return { name: "Bilbo" };
    },
  })
  .mutation("torrents.add", {
    // validate input with Zod
    input: z.object({ name: z.string().min(5) }),
    async resolve(req) {
    },
  });

export type AppRouter = typeof appRouter;
