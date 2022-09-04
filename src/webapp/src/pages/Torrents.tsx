import { Box, CircularProgress, CircularProgressLabel, Flex, Icon, IconButton, Input, Menu, MenuButton, MenuGroup, MenuItem, MenuList, Table, Tbody, Td, Th, Thead, Tr } from "@chakra-ui/react";
import React from "react";
import { MdCheck, MdDriveFileMove, MdMenu, MdOutlineFolder, MdPause, MdPlayArrow, MdRemove } from "react-icons/md";
import { TbUpload } from "react-icons/tb";
import { trpc } from "../utils/trpc";
import filesize from "filesize";

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
  const torrents = trpc.useQuery(["torrents.list"], {
    refetchInterval: 1000
  });

  const pause = trpc.useMutation(["torrents.pause"]);
  const resume = trpc.useMutation(["torrents.resume"]);

  if (!torrents.data) {
    return <div>Loading</div>
  }

  return (
    <>
      <Flex
        backgroundColor={"#fff"}
        border="1px solid #f0f0f0"
        borderRadius={4}
        marginBottom={5}
        padding={3}
      >
        <Input
          marginRight={3}
          placeholder="Filter torrent list..."
        />
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
              <Th>Name</Th>
              <Th textAlign={"right"}>Size</Th>
              <Th textAlign={"right"}>DL</Th>
              <Th textAlign={"right"}>UL</Th>
              <Th textAlign={"right"}>Peers</Th>
              <Th textAlign={"right"} w={"16px"}></Th>
            </Tr>
          </Thead>
          <Tbody>
            { torrents.data.map((t,idx) => (
              <Tr key={idx}>
                <Td
                  paddingEnd={0}
                >
                  <CircularProgress
                    color={getColorFromState(t.state)}
                    size={"32px"}
                    value={t.progress*100}
                  >
                    <CircularProgressLabel display={"flex"} alignItems={"center"} justifyContent={"center"}>
                      <ProgressLabel torrent={t} />
                    </CircularProgressLabel>
                  </CircularProgress>
                </Td>
                <Td>
                  <Box>
                    {t.name}
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
                    {t.save_path}
                  </Flex>
                </Td>
                <Td textAlign={"right"}>{filesize(t.size)}</Td>
                <Td textAlign={"right"}>{t.download_rate < 1024 ? "-" : filesize(t.download_rate)+"/s"}</Td>
                <Td textAlign={"right"}>{t.upload_rate   < 1024 ? "-" : filesize(t.upload_rate)+"/s"}</Td>
                <Td textAlign={"right"}>{t.num_peers === 0 ? "-" : t.num_peers}</Td>
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
                      <MenuGroup title="Actions">
                        { isPaused(t.flags)
                          ? <MenuItem
                              icon={<MdPlayArrow />}
                              onClick={async () => {
                                await resume.mutateAsync(t.info_hash)
                              }}
                            >
                              Resume
                            </MenuItem>
                          : <MenuItem
                              icon={<MdPause />}
                              onClick={async () => {
                                await pause.mutateAsync(t.info_hash);
                              }}
                            >
                              Pause
                            </MenuItem>
                        }
                        <MenuItem
                          icon={<MdDriveFileMove />}
                        >
                          Move
                        </MenuItem>
                        <MenuItem
                          icon={<MdRemove />}
                        >
                          Remove
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
