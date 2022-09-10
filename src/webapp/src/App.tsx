import "./scss/app.scss";
import React, { useState } from "react";
import { Outlet } from "react-router-dom";
import { QueryClient, QueryClientProvider } from "react-query";
import { trpc } from "./utils/trpc";
import { Box, Flex } from "@chakra-ui/react";
import Sidebar from "./components/Sidebar";
import "./i18n";

function App() {
  const [queryClient] = useState(() => new QueryClient());
  const [trpcClient] = useState(() =>
    trpc.createClient({ url: "/trpc" }));

  return (
    <trpc.Provider client={trpcClient} queryClient={queryClient}>
      <QueryClientProvider client={queryClient}>
        <Flex height={"100vh"}>
          <Sidebar />
          <Box flex={1} p={5}>
            <Outlet />
          </Box>
        </Flex>
      </QueryClientProvider>
    </trpc.Provider>
  )
}

export default App;
