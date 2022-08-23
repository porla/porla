import React, { useState } from "react";
import { QueryClient, QueryClientProvider } from "react-query";
import { NavLink, Outlet } from "react-router-dom";
import { VscAdd, VscFiles, VscMenu, VscSettingsGear } from "react-icons/vsc";
import { trpc } from "./utils/trpc";

import "./App.css";
import { Box, Button, Divider, Flex, Heading, IconButton, Image, Input, Link, List, ListIcon, ListItem, Text } from "@chakra-ui/react";

function App() {
  const [queryClient] = useState(() => new QueryClient());
  const [trpcClient] = useState(() =>
    trpc.createClient({ url: "/trpc" }));

  let activeClassName = "underline";
  return (
    <trpc.Provider client={trpcClient} queryClient={queryClient}>
      <QueryClientProvider client={queryClient}>
        <Flex
          height={"100vh"}
        >
          <Flex
            borderRight={"1px solid #f0f0f0"}
            boxShadow={"3px 0 3px #fefefe"}
            direction="column"
            justifyContent={"flex-end"}
            height={"100%"}
            width={"200px"}
            padding={5}
          >
            <Box
              flex={1}
            >
              <Image
                marginX="auto"
                maxWidth={"96px"}
                src="/porla_logo.png"
              />

              <Divider marginY={3} />

              <List>
                <ListItem
                >
                  <Box>
                    <Flex
                      alignItems={"center"}
                    >
                      <ListIcon
                        as={VscFiles}
                      />
                      <Link
                        _activeLink={{
                          fontWeight: "bold"
                        }}
                        as={NavLink}
                        flex={1}
                        to="/"
                      >
                        Torrents
                      </Link>
                      <IconButton
                        aria-label=""
                        as={NavLink}
                        icon={<VscAdd />}
                        size="xs"
                        to="/add"
                      />
                    </Flex>
                  </Box>
                </ListItem>
              </List>
            </Box>
            <Box
            >
              <Divider marginY={3} />
              <List>
                <ListItem>
                  <ListIcon
                    as={VscSettingsGear}
                  />
                  <Link
                    _activeLink={{
                      fontWeight: "bold"
                    }}
                    as={NavLink}
                    flex={1}
                    to="/settings"
                  >
                    Settings
                  </Link>
                </ListItem>
              </List>
            </Box>
          </Flex>
          <Box
            backgroundColor={"#fbfbfb"}
            flex={1}
            padding={5}
          >
            <Outlet />
          </Box>
        </Flex>
      </QueryClientProvider>
    </trpc.Provider>
  )
}

export default App;
