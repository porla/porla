import { createFileRoute, redirect } from '@tanstack/react-router'
import Index from "../../Index.tsx";

export const Route = createFileRoute('/_auth/_indexLayout')({

  beforeLoad: async ({ context, location }) => {
    if (!context.auth.isAuthenticated) {
      throw redirect({
        to: '/login',
        search: {
          // Use the current location to power a redirect after login
          // (Do not use `router.state.resolvedLocation` as it can
          // potentially lag behind the actual current location)
          redirect: location.href,
        },
      })
    }
  },

  component: Index,

})