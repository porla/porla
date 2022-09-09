import "./scss/App.scss";
import React, { useState } from "react";
import { QueryClient, QueryClientProvider } from "react-query";
import { NavLink, Outlet } from "react-router-dom";
import { VscAdd, VscDashboard, VscFiles, VscMenu, VscSettingsGear } from "react-icons/vsc";
import { trpc } from "./utils/trpc";
import { Box, Button, ColorModeScript, Divider, Flex, Heading, IconButton, Image, Input, Link, List, ListIcon, ListItem, Text } from "@chakra-ui/react";
import "./i18n";
import { useTranslation } from "react-i18next";
import theme from "./theme";

function App() {
  const { t } = useTranslation();
  const [queryClient] = useState(() => new QueryClient());
  const [trpcClient] = useState(() =>
    trpc.createClient({ url: "/trpc" }
  ));

  return (
    <trpc.Provider client={trpcClient} queryClient={queryClient}>
      <QueryClientProvider client={queryClient}>
        <ColorModeScript initialColorMode={theme.config.initialColorMode} />
        <Flex
          height={"100vh"}
        >
          <Flex
            borderRightWidth={1}
            direction="column"
            justifyContent="flex-end"
            height="100%"
            width="250px"
            padding={5}
          >
            <Box
              flex={1}
            >
              <Image
                maxW="64px"
                marginX="auto"
                src="/brand/porla_isotype.svg"
              />

              <Divider marginY={5} />

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
                <ListItem>
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
            <Divider marginY={3} />
            <Box
            >
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
