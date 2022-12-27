import { Box, CircularProgress, CircularProgressLabel, Flex, Grid, GridItem, HStack, Icon, IconButton, Link, Menu, MenuButton, MenuDivider, MenuGroup, MenuItem, MenuList, Text, textDecoration, useColorMode } from "@chakra-ui/react";
import { filesize } from "filesize";
import { IconType } from "react-icons/lib";
import { MdCheck, MdDelete, MdDriveFileMove, MdFileCopy, MdFolder, MdFolderOpen, MdLabel, MdOutlineMoreVert, MdOutlineReport, MdPause, MdPlayArrow, MdSchedule, MdSearch, MdTag, MdUpload, MdViewList } from "react-icons/md";
import { Torrent } from "../types"

import useNinja from "../contexts/ninja";
import useTorrentsFilter from "../contexts/TorrentsFilterContext";

type TorrentsListItemProps = {
  index: number;
  isDeleting: (torrent: Torrent) => boolean;
  onMove: (torrent: Torrent) => void;
  onPause: (torrent: Torrent) => void;
  onRecheck: (torrent: Torrent) => void;
  onRemove: (torrent: Torrent) => void;
  onResume: (torrent: Torrent) => void;
  onShowProperties: (torrent: Torrent) => void;
  torrent: Torrent;
}

type KeyValueProps = {
  icon: IconType;
  value: any;
}

function checkBit(flags: number, bit: number) {
  return (flags & (1<<bit)) === 1<<bit;
}

function isAutoManaged(flags: number) {
  return checkBit(flags, 5);
}

function isPaused(flags: number) {
  return (flags & (1<<4)) === 1<<4;
}

function randomLinuxTorrent(index: number) {
  const pool = [
    "archlinux-2022.10.01-x86_64.iso",
    "archlinux-2022.11.01-x86_64.iso",
    "archlinux-2022.12.01-x86_64.iso",
    "debian-11.6.0-amd64-netinst.iso",
    "debian-edu-11.6.0-amd64-netinst.iso",
    "debian-mac-11.6.0-amd64-netinst.iso",
    "ubuntu-20.04.5-desktop-amd64.iso",
    "ubuntu-20.04.5-live-server-amd64.iso",
    "ubuntu-22.04.1-desktop-amd64.iso",
    "ubuntu-22.04.1-live-server-amd64.iso"
  ];

  return pool[index % pool.length];
}

function stateColor(torrent: Torrent) {
  if (torrent.error) {
    return "red";
  }

  if (torrent.moving_storage) {
    return "yellow.400";
  }

  switch (torrent.state) {
    case 1: return "gray.500";
    case 2: return "gray.600";
    case 3: return "blue.500";
    case 5: return "green.300";
  }
  return "default";
}

function eta(seconds: number) {
  let m = Math.floor(seconds / 60);
  let s = seconds % 60;
  let h = Math.floor(m / 60);
  m = m % 60;
  h = h % 24;

  if (h > 0) {
    return `${h}h ${m}m`;
  }

  if (m > 0) {
    return `${m}m ${s}s`;
  }

  return `${s}s`;
}

function progressLabel(torrent: Torrent) {
  const w = 4;
  const h = 4;

  if (torrent.error) {
    return "error";
  }

  if (torrent.moving_storage) {
    return <Icon as={MdDriveFileMove} mt={"3px"} w={w} h={h} />;
  }

  switch (torrent.state) {
    case 1: {
      if (isPaused(torrent.flags)) {
        return "checking_files_queued";
      }
      break;
    }
    case 2: return <Icon as={isAutoManaged(torrent.flags) && isPaused(torrent.flags) ? MdSchedule : MdFileCopy} mt={"1px"} w={3} h={3} />;
    case 3: {
      if (isPaused(torrent.flags)) {
        if (isAutoManaged(torrent.flags)) {
          return <Icon as={MdSchedule} mt={"1px"} w={3} h={3} />;
        }
        return "paused";
      }
      break;
    }
    case 4: return <Icon as={MdCheck} mt={"1px"} w={3} h={3} />;
    case 5: {
      if (isPaused(torrent.flags)) {
        if (isAutoManaged(torrent.flags)) {
          return "seeding_queued";
        }
        return <Icon as={MdCheck} mt={"1px"} w={3} h={3} />;
      }
      return <Icon as={MdUpload} mt={"3px"} w={4} h={4} />;
    }
  }
  return Math.round(torrent.progress * 100);
}

function KeyValue(props: KeyValueProps) {
  return (
    <Flex
      alignItems={"center"}
    >
      <Icon
        as={props.icon}
      />
      <Box ms={1}>{props.value}</Box>
    </Flex>
  )
}

export default function TorrentsListItem(props: TorrentsListItemProps) {
  const { colorMode } = useColorMode();
  const { isNinja } = useNinja();
  const { addFilter } = useTorrentsFilter();

  return (
    <Grid
      gridTemplateColumns={"32px 24px 1fr 100px 110px 110px 100px 48px"}
      gridTemplateRows={"min-content"}
    >
      <GridItem alignSelf={"center"}>
        {
          props.torrent.error
            ? <Icon
                as={MdOutlineReport}
                color={"red.300"}
                h={"32px"}
                w={"32px"}
              />
            : <CircularProgress
                color={stateColor(props.torrent)}
                isIndeterminate={
                  (props.torrent.state === 2
                    && isAutoManaged(props.torrent.flags)
                    && !isPaused(props.torrent.flags)) || props.torrent.moving_storage
                }
                size={"32px"}
                trackColor={""}
                value={props.torrent.progress * 100}
              >
                <CircularProgressLabel fontSize={"xs"}>{progressLabel(props.torrent)}</CircularProgressLabel>
              </CircularProgress>
        }
      </GridItem>

      <GridItem alignSelf={"center"} textAlign={"center"}>
        <Text
          color={"gray.500"}
          fontSize={"xs"}
          ms={2}
        >
          {props.torrent.queue_position < 0 ? "-" : props.torrent.queue_position + 1}
        </Text>
      </GridItem>

      <GridItem m={2} overflow={"hidden"}>
        <Text
          overflow={"hidden"}
          overflowWrap={"anywhere"}
          textOverflow={"ellipsis"}
          whiteSpace={"nowrap"}
          wordBreak={"break-all"}
        >
          {isNinja ? randomLinuxTorrent(props.index) : props.torrent.name}
        </Text>
        <HStack
        >
          <HStack
            color={colorMode === "dark" ? "whiteAlpha.600" : "blackAlpha.600"}
            fontSize={"sm"}
            spacing={3}
          >
            <KeyValue
              key={"savepath"}
              icon={MdFolderOpen}
              value={isNinja
                ? "/legit/linux/isos"
                : <Link
                    _hover={{
                      color: "white",
                      textDecoration: "underline"
                    }}
                    onClick={() => addFilter({ field: "save_path", args: props.torrent.save_path })}
                  >
                    {props.torrent.save_path}
                  </Link>}
            />

            {props.torrent.size > 0 && (
              <KeyValue key={"size"} icon={MdFileCopy} value={filesize(props.torrent.size).toString()} />
            )}
            { props.torrent.category
              && <KeyValue icon={MdLabel} value={
                <Link
                  _hover={{
                    color: "white",
                    textDecoration: "underline"
                  }}
                  onClick={() => addFilter({ field: "category", args: props.torrent.category! })}
                >
                  {props.torrent.category}
                </Link>
              } />}
            {
              props.torrent.tags.length > 0
                && <KeyValue key={"tags"} icon={MdTag} value={
                    <Flex alignItems={"center"}>
                      {props.torrent.tags.map(t => (
                        <Link
                          _hover={{
                            color: "white",
                            textDecoration: "underline"
                          }}
                          key={t}
                          me={1}
                          onClick={() => addFilter({ field: "tags", args: t})}
                        >
                          {t}
                        </Link>
                      ))}
                    </Flex>
                  } />
            }
          </HStack>
        </HStack>
      </GridItem>

      <GridItem alignSelf={"center"} textAlign={"end"}>
        <Text
          color={props.torrent.eta > 0 ? "" : "gray.500"}
          fontSize={"sm"}
          mx={2}
        >
          {props.torrent.eta > 0 ? eta(props.torrent.eta) : "-"}
        </Text>
      </GridItem>

      <GridItem alignSelf={"center"} textAlign={"end"}>
        <Text
          fontSize={"sm"}
          mx={2}
        >
          {props.torrent.ratio.toFixed(2)}
        </Text>
      </GridItem>

      <GridItem alignSelf={"center"} textAlign={"end"}>
        <Text
          color={props.torrent.download_rate > 1024 ? "" : "gray.500"}
          fontSize={"sm"}
          mx={2}
        >
          {props.torrent.download_rate > 1024
            ? <>{filesize(props.torrent.download_rate).toString()}/s</>
            : <>-</>
          }
        </Text>
      </GridItem>

      <GridItem alignSelf={"center"} textAlign={"end"}>
        <Text
          color={props.torrent.upload_rate > 1024 ? "" : "gray.500"}
          fontSize={"sm"}
          mx={2}
        >
          {props.torrent.upload_rate > 1024
            ? <>{filesize(props.torrent.upload_rate).toString()}/s</>
            : <>-</>
          }
        </Text>
      </GridItem>

      <GridItem alignSelf={"center"}>
        <Flex justifyContent={"end"}>
          <Menu>
            <MenuButton
              as={IconButton}
              icon={<MdOutlineMoreVert />}
              size={"sm"}
            />
            <MenuList>
              {
                isPaused(props.torrent.flags)
                  ? <MenuItem
                      icon={<MdPlayArrow />}
                      onClick={() => props.onResume(props.torrent)}
                    >
                      Resume
                    </MenuItem>
                  : <MenuItem
                      icon={<MdPause />}
                      onClick={() => props.onPause(props.torrent)}
                    >
                      Pause
                    </MenuItem>
              }
              <MenuDivider />
              <MenuGroup title="Actions">
                <MenuItem
                  icon={<MdSearch />}
                  onClick={() => props.onRecheck(props.torrent)}
                >
                  Check files
                </MenuItem>
                <MenuItem
                  icon={<MdFolder />}
                  onClick={() => props.onMove(props.torrent)}
                >
                  Move contents
                </MenuItem>
                <MenuItem
                  icon={<MdDelete />}
                  onClick={() => props.onRemove(props.torrent)}
                >
                  Remove torrent
                </MenuItem>
              </MenuGroup>
              <MenuDivider />
              <MenuItem
                icon={<MdViewList />}
                onClick={() => props.onShowProperties(props.torrent)}
              >
                Properties
              </MenuItem>
            </MenuList>
          </Menu>
        </Flex>
      </GridItem>
    </Grid>
  )
}
