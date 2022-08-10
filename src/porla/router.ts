import * as trpc from "@trpc/server";
import { z } from "zod";

export const appRouter = trpc
  .router()
  .query("torrents.list", {
    async resolve(req) {
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
