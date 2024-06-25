import { Flex, Grid, GridItem, HStack, IconButton, Image, Link, Spacer, Stack, Text, useColorMode } from '@chakra-ui/react';
import { useEffect, useState } from 'react';
import { MdDarkMode, MdExtension, MdLightMode, MdOutlineCameraAlt } from 'react-icons/md';
import { SiDiscord } from 'react-icons/si';
import { Navigate, NavLink, Outlet } from 'react-router-dom';

import Isotype from './assets/isotype.svg';
import prefixPath from './base';
import AppErrorModal from './components/AppErrorModal';
import Query from './components/Query';
import SettingsDrawer from './components/SettingsDrawer';
import useAuth from './contexts/auth';
import useNinja from './contexts/ninja';
import { AuthError, useRPC } from './services/jsonrpc';

type IVersionInfo = {
  porla: {
    commitish: string;
    branch: string;
    version: string;
  }
}

function AuthApp() {
  const { data, error } = useRPC<IVersionInfo>("sys.versions");
  const [ showSettings, setShowSettings ] = useState(false);
  const { colorMode, toggleColorMode } = useColorMode();
  const { isNinja, toggleNinja } = useNinja();

  if (error && error instanceof AuthError) {
    return <Navigate to="/login" />
  }

  if (error) {
    return (
      <AppErrorModal error={error} />
    );
  }

  return (
    <>
      <SettingsDrawer
        isOpen={showSettings}
        onClose={() => setShowSettings(false)}
      />

      <Grid
        height={"100vh"}
        templateAreas={`
          "header"
          "main"
          "footer"
        `}
        gridTemplateColumns={"1fr"}
        gridTemplateRows={"0fr 1fr 0fr"}
      >
        <GridItem area={"header"}>
          <Flex
            borderBottomColor={"whiteAlpha.400"}
            borderBottomWidth={"1px"}
            boxShadow={"sm"}
            justifyContent={"space-between"}
            mb={2}
            p={3}
          >
            <Stack direction={"row"} alignItems={"center"} spacing={5}>
              <NavLink to="/">
                <Image src={Isotype} width={"32px"} />
              </NavLink>
            </Stack>

            <Query />

            <Flex>
              <IconButton
                aria-label="Plugins"
                icon={<MdExtension />}
                variant={"link"}
                as={NavLink}
                to="/plugins"
              />
            </Flex>
          </Flex>
        </GridItem>

        <GridItem area={"main"} overflow={"auto"}>
          <Outlet />
        </GridItem>

        <GridItem area={"footer"}>
          <HStack
            justifyContent={"end"}
            padding={1}
            backgroundColor={colorMode === "dark" ? "whiteAlpha.50" : ""}
            borderTopColor={colorMode === "dark" ? "whiteAlpha.200" : ""}
            borderTopWidth={"1px"}
            color={colorMode === "dark" ? "whiteAlpha.600" : ""}
          >
            <IconButton
              aria-label={""}
              icon={colorMode === "dark" ? <MdLightMode /> : <MdDarkMode />}
              onClick={() => toggleColorMode()}
              size={"xs"}
              variant={"link"}
            />

            <IconButton
              aria-label={"Join Porla on Discord"}
              as={Link}
              href="https://discord.gg/FNVVRzqJAG"
              icon={<SiDiscord />}
              isExternal
              size={"xs"}
              title={"Join Porla on Discord"}
              variant={"link"}
            />

            <IconButton
              aria-label={"Toggle Ninja mode"}
              color={isNinja ? "red.400" : ""}
              icon={<MdOutlineCameraAlt />}
              onClick={() => toggleNinja()}
              size={"xs"}
              variant={"link"}
            />

            <Spacer />
            <Text fontSize={"xs"}>{data?.porla.version === "" ? "dev" : data?.porla.version}</Text>
            { data?.porla.branch    !== "-" && <Text fontSize={"xs"}>{data?.porla.branch}</Text> }
            { data?.porla.commitish !== "-" && <Text fontSize={"xs"}>{data?.porla.commitish.substring(0,7)}</Text> }
          </HStack>
        </GridItem>
      </Grid>
    </>
  )
}

export default function App() {
  const { user } = useAuth();
  const [status, setStatus] = useState<string | undefined>();

  useEffect(() => {
    fetch(prefixPath("/api/v1/system"))
      .then(r => r.json())
      .then(s => {
        setStatus(s.status);
      });
  }, []);

  if (!status) {
    return <div>Loading</div>
  }

  if (status === "setup") {
    return <Navigate to={"/setup"} />
  }

  if (!user) {
    return <Navigate to={"/login"} />
  }

  return <AuthApp />
}
