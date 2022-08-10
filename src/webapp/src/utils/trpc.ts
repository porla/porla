import { createReactQueryHooks } from "@trpc/react";
import type { AppRouter } from "../../../porla/router";

export const trpc = createReactQueryHooks<AppRouter>();
