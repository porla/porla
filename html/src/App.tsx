import { Box, Flex, HStack, IconButton, Image, Link, Spacer, Stack, Text, useColorMode } from '@chakra-ui/react';
import { useState } from 'react';
import { MdDarkMode, MdHelp, MdLightMode, MdSettings } from 'react-icons/md';
import { SiDiscord } from 'react-icons/si';
import { Outlet } from 'react-router-dom';

import Isotype from './assets/isotype.svg';
import AppErrorModal from './components/AppErrorModal';
import SettingsDrawer from './components/SettingsDrawer';
import { useRPC } from './services/jsonrpc';

type IVersionInfo = {
  porla: {
    commitish: string;
    branch: string;
    version: string;
  }
}

export default function App() {
  const { data, error } = useRPC<IVersionInfo>("sys.versions");
  const [ showSettings, setShowSettings ] = useState(false);
  const { colorMode, toggleColorMode } = useColorMode();

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