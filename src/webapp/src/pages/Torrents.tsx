import React, { useState } from "react";
import { useTranslation } from "react-i18next";
import { Box, CircularProgress, CircularProgressLabel, Flex, Icon, IconButton, Input, InputLeftElement, InputGroup, Menu, MenuButton, MenuGroup, MenuItem, MenuList, Table, Tbody, Td, Th, Thead, Tr, useDisclosure } from "@chakra-ui/react";
import { MdFilterList, MdCheck, MdDriveFileMove, MdMenu, MdOutlineFolder, MdDelete, MdPause, MdPlayArrow } from "react-icons/md";
import { TbUpload } from "react-icons/tb";
import { trpc } from "../utils/trpc";
import filesize from "filesize";
import Loading from "../components/Loading";
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

  const torrents = trpc.useQuery(["torrents.list"], {
    refetchInterval: 1000
  });

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

      <Flex
        backgroundColor={"#fff"}
        border="1px solid #f0f0f0"
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
            marginRight={3}
            placeholder={t("filter_torrent_list")}
          />
        </InputGroup>
        <IconButton
          aria-label=""
          icon={<MdMenu />}
        />
      </Flex>
      <Box
        backgroundColor={"#fff"}
        border="1px solid #f0f0f0"
        borderRadius={4}
        padding={3}
      >
        <Table>
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
            { torrents.data.map((item, idx) => (
              <Tr key={idx}>
                <Td
                  paddingEnd={0}
                >
                  <CircularProgress
                    color={getColorFromState(item.state)}
                    size={"32px"}
                    value={item.progress*100}
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
                <Td
                  paddingEnd={0}
                >
                  <Menu>
                    <MenuButton
                      as={IconButton}
                      icon={<MdMenu />}
                      size={"sm"}
                      variant={"ghost"}
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
      </Box>
    </>
  );
}

export default Torrents;
