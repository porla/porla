import { AuthProvider, useAuth } from "./auth.tsx";
import { RouterProvider } from "@tanstack/react-router";

function InnerApp() {
  const auth = useAuth()
  return <RouterProvider router={router} context={{ auth }} />
}

export function App() {
  return (
    <AuthProvider>
      <InnerApp />
    </AuthProvider>
  )
}