import "./App.css";
import React, { useState } from "react";
import { QueryClient, QueryClientProvider } from "react-query";
import { NavLink, Outlet } from "react-router-dom";
import { VscAdd, VscDashboard, VscFiles, VscMenu, VscSettingsGear } from "react-icons/vsc";
import { trpc } from "./utils/trpc";
import { Box, Button, Divider, Flex, Heading, IconButton, Image, Input, Link, List, ListIcon, ListItem, Text } from "@chakra-ui/react";
import "../../../i18n";
import { useTranslation } from "react-i18next";

function App() {
  const { t } = useTranslation();
  const [queryClient] = useState(() => new QueryClient());
  const [trpcClient] = useState(() =>
    trpc.createClient({ url: "/trpc" }
  ));

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
                paddingX={5}
                src="/porla_isotype.svg"
              />

              <Divider marginY={3} />

              <List>
                <ListItem mb={3}>
                  <ListIcon
                    as={VscDashboard}
                  />
                  <Link
                    _activeLink={{
                      fontWeight: "bold"
                    }}
                    as={NavLink}
                    flex={1}
                    to="/"
                  >
                    {t("home")}
                  </Link>
                </ListItem>
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
                        to="/torrents"
                      >
                        {t("torrents")}
                      </Link>
                      <IconButton
                        aria-label=""
                        as={NavLink}
                        icon={<VscAdd />}
                        size="xs"
                        to="/torrents/add"
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
                <ListItem display={"flex"} alignItems={"center"}>
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
                    {t("settings")}
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
