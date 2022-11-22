import { Box, Flex, HStack, IconButton, Image, Link, Spacer, Stack, Text, useColorMode } from '@chakra-ui/react';
import { useEffect, useState } from 'react';
import { MdDarkMode, MdLightMode, MdSettings } from 'react-icons/md';
import { SiDiscord } from 'react-icons/si';
import { Navigate, Outlet } from 'react-router-dom';

import Isotype from './assets/isotype.svg';
import AppErrorModal from './components/AppErrorModal';
import SettingsDrawer from './components/SettingsDrawer';
import useAuth from './contexts/auth';
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

  if (error && error instanceof AuthError) {
    return <Navigate to="/login" />
  }

  if (error) {
    return (
      <AppErrorModal error={error} />
    );
  }

  return (
    <Flex direction={"column"} height={"100%"} mx={"auto"}>
      <SettingsDrawer
        isOpen={showSettings}
        onClose={() => setShowSettings(false)}
      />

      <Flex
        borderBottomColor={"whiteAlpha.400"}
        borderBottomWidth={"1px"}
        boxShadow={"sm"}
        justifyContent={"space-between"}
        mb={2}
        p={3}
      >
        <Stack direction={"row"} alignItems={"center"} spacing={5}>
          <Image src={Isotype} width={"32px"} />
        </Stack>

        <IconButton
          aria-label=''
          icon={<MdSettings />}
          onClick={() => setShowSettings(true)}
          variant={"link"}
        />
      </Flex>
      <Box flex={1}>
        <Outlet />
      </Box>
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
        <Spacer />
        <Text fontSize={"xs"}>{data?.porla.version === "" ? "dev" : data?.porla.version}</Text>
        <Text fontSize={"xs"}>{data?.porla.branch}</Text>
        <Text fontSize={"xs"}>{data?.porla.commitish.substring(0,7)}</Text>
      </HStack>
    </Flex>
  )
}

export default function App() {
  const { user } = useAuth();
  const [status, setStatus] = useState<string | undefined>();

  useEffect(() => {
    fetch("/api/v1/system")
      .then(r => r.json())
      .then(s => {
        setStatus(s.status);
      });
  }, []);

  if (!status) {
    return <div>Loading</div>
  }

  console.log(status, user);

  if (status === "setup") {
    return <Navigate to={"/setup"} />
  }

  if (!user) {
    console.log('redir to login');
    return <Navigate to={"/login"} />
  }

  return <AuthApp />
}
