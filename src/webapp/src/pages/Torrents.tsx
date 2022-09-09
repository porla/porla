import "../scss/pages/Torrents.scss";
import React, { useState } from "react";
import { useTranslation, Trans } from "react-i18next";
import {
  Box, Center, CircularProgress, CircularProgressLabel, Flex, HStack, Icon, IconButton, Input, Link, InputLeftElement, InputGroup, Menu, MenuButton, MenuGroup, MenuItem, MenuList, Text,
  Table, Tbody, Td, Th, Thead, Tr,
  useColorMode, useDisclosure,
} from "@chakra-ui/react";
import { MdFilterList, MdCheck, MdDriveFileMove, MdTableView, MdMenu, MdOutlineFolder, MdDelete, MdPause, MdPlayArrow, } from "react-icons/md";
import { TbUpload } from "react-icons/tb";
import { trpc } from "../utils/trpc";
import filesize from "filesize";
import Loading from "../components/Loading";
import ToggleTheme from "../components/ToggleTheme";
import { MoveTorrentModal } from "../components/MoveTorrentModal";
import { RemoveTorrentDialog } from "../components/RemoveTorrentDialog";

function getColorFromState(state: number): string {
  switch (state) {
    case 3: return "blue.400";
    case 5: return "green.400";
  }
  return "black";
}

function isCompleted(torrent: {state: number, flags: number}): boolean {
  return torrent.state === 5 && isPaused(torrent.flags);
}

function isPaused(flags: number): boolean {
  return (flags & (1<<4)) === 1<<4;
}

function ProgressLabel({ torrent }: any) {
  if (torrent.progress === 1 && isCompleted(torrent)) {
    return (
      <Icon
        as={MdCheck}
        color={"green.700"}
        fontSize={"sm"}
      />
    )
  }

  if (torrent.progress === 1) {
    return (
      <Icon
        as={TbUpload}
        color={"green.700"}
        fontSize={"sm"}
      />
    )
  }

  if (isPaused(torrent.flags)) {
    return (
      <Icon
        as={MdPause}
        color={"green.700"}
        fontSize={"sm"}
      />
    )
  }

  return (
    <>{Math.trunc(torrent.progress * 100)}%</>
  )
}

function Torrents() {
  const { t } = useTranslation();
  const { colorMode } = useColorMode();

  const torrents = trpc.useQuery(["torrents.list"], {
    refetchInterval: 1000
  });

  const [tableSize, setTableSize] = useState('md');
  const [selectedTorrent, setSelectedTorrent] = useState({});

  const pause = trpc.useMutation(["torrents.pause"]);
  const resume = trpc.useMutation(["torrents.resume"]);

  const {
    isOpen: move_isOpen,
    onClose: move_onClose,
    onOpen: move_onOpen
  } = useDisclosure();

  const {
    isOpen: remove_isOpen,
    onClose: remove_onClose,
    onOpen: remove_onOpen
  } = useDisclosure();

  const changeTableSize = () => {
    switch (tableSize) {
      case "sm": setTableSize("md"); break;
      case "md": setTableSize("lg"); break;
      case "lg": setTableSize("sm"); break;
      default: setTableSize("md"); break;
    }
  }

  if (!torrents.data) {
    return <Loading />
  }

  return (
    <>
      <MoveTorrentModal
        isOpen={move_isOpen}
        onClose={move_onClose}
        onOpen={move_onOpen}
        torrent={selectedTorrent} />

      <RemoveTorrentDialog
        isOpen={remove_isOpen}
        onClose={remove_onClose}
        onOpen={remove_onOpen}
        torrent={selectedTorrent}
      />

      <HStack
        borderWidth={1}
        borderRadius={4}
        marginBottom={5}
        padding={3}
      >
        <InputGroup>
          <InputLeftElement
            pointerEvents='none'
            children={<MdFilterList />}
          />
          <Input
            placeholder={t("filter_torrent_list")}
          />
        </InputGroup>
        <IconButton
          aria-label=""
          icon={<MdTableView />}
          onClick={changeTableSize}
        />
        <ToggleTheme/>
        <IconButton
          aria-label=""
          icon={<MdMenu />}
        />
      </HStack>
      <Box
        borderWidth={1}
        borderRadius={4}
        padding={0}
      >
        {torrents.data && torrents.data.length > 0 && (
          <Table className="table-torrents" size={tableSize}>
            <Thead>
              <Tr>
                <Th w={"16px"}></Th>
                <Th>{t("name")}</Th>
                <Th textAlign={"right"}>{t("size")}</Th>
                <Th textAlign={"right"}>{t("dl")}</Th>
                <Th textAlign={"right"}>{t("ul")}</Th>
                <Th textAlign={"right"}>{t("peers")}</Th>
                <Th textAlign={"right"} w={"16px"}></Th>
              </Tr>
            </Thead>
            <Tbody>
              {torrents.data.map((item, idx) => (
                <Tr key={idx}>
                  <Td
                    paddingEnd={0}
                  >
                    <CircularProgress
                      color={getColorFromState(item.state)}
                      value={item.progress*100}
                      size="32px"
                      thickness="15px"
                      trackColor={colorMode === "light" ? "blackAlpha.300" : "whiteAlpha.300"}
                    >
                      <CircularProgressLabel display={"flex"} alignItems={"center"} justifyContent={"center"}>
                        <ProgressLabel torrent={item} />
                      </CircularProgressLabel>
                    </CircularProgress>
                  </Td>
                  <Td>
                    <Box>
                      {item.name}
                    </Box>
                    <Flex
                      alignItems={"center"}
                      fontSize={"xs"}
                      opacity={.5}
                    >
                      <Icon
                        as={MdOutlineFolder}
                        mr={1}
                        fontSize={"sm"}
                      />
                      {item.save_path}
                    </Flex>
                  </Td>
                  <Td textAlign={"right"}>{filesize(item.size)}</Td>
                  <Td textAlign={"right"}>{item.download_payload_rate < 1024 ? "-" : filesize(item.download_payload_rate)+"/s"}</Td>
                  <Td textAlign={"right"}>{item.upload_payload_rate   < 1024 ? "-" : filesize(item.upload_payload_rate)+"/s"}</Td>
                  <Td textAlign={"right"}>{item.num_peers === 0 ? "-" : item.num_peers}</Td>
                  <Td>
                    <Menu>
                      <MenuButton
                        as={IconButton}
                        icon={<MdMenu />}
                        size="sm"
                        variant="ghost"
                      />
                      <MenuList>
                        <MenuGroup title={t("actions")}>
                          { isPaused(item.flags)
                            ? <MenuItem
                                icon={<MdPlayArrow />}
                                onClick={async () => {
                                  await resume.mutateAsync(item.info_hash)
                                }}
                              >
                                {t("resume")}
                              </MenuItem>
                            : <MenuItem
                                icon={<MdPause />}
                                onClick={async () => {
                                  await pause.mutateAsync(item.info_hash);
                                }}
                              >
                                {t("pause")}
                              </MenuItem>
                          }
                          <MenuItem
                            icon={<MdDriveFileMove />}
                            onClick={() => {
                              setSelectedTorrent(item);
                              move_onOpen();
                            }}
                          >
                            {t("move")}
                          </MenuItem>
                          <MenuItem
                            icon={<MdDelete />}
                            onClick={() => {
                              setSelectedTorrent(item);
                              remove_onOpen();
                            }}
                          >
                            {t("remove")}
                          </MenuItem>
                        </MenuGroup>
                      </MenuList>
                    </Menu>
                  </Td>
                </Tr>
              ))}
            </Tbody>
          </Table>
        )}
        {torrents.data.length === 0 && (
          <Center padding={10}>
            <Text fontSize="lg">
              <Trans i18nKey="no_torrents" components={[<Link href="/torrents/add"/>]}/>
            </Text>
          </Center>
        )}
      </Box>
    </>
  );
}

export default Torrents;
