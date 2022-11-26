import { Table, Thead, Tr, Th, Tbody, Td, Badge, Menu, MenuButton, IconButton, MenuList, MenuGroup, MenuItem, Text, Icon, HStack, Box, Tooltip } from "@chakra-ui/react";
import { filesize } from "filesize";
import { MdOutlineMoreVert, MdDelete, MdFolder, MdWarning } from "react-icons/md";

function checkBit(flags: number, bit: number) {
  return (flags & (1<<bit)) === 1<<bit;
}

function isAutoManaged(flags: number) {
  return checkBit(flags, 5);
}

function isPaused(flags: number) {
  return (flags & (1<<4)) === 1<<4;
}

function stateColor(torrent: any) {
  if (torrent.error) {
    return "red";
  }

  switch (torrent.state) {
    case 3: return "blue";
    case 5: return "green";
  }
  return "default";
}

function stateString(torrent: any) {
  if (torrent.error) {
    return "error";
  }

  switch (torrent.state) {
    case 1: {
      if (isPaused(torrent.flags)) {
        return "checking_files_queued";
      }
      return "checking_files";
    }
    case 2: return "downloading_metadata";
    case 3: {
      if (isPaused(torrent.flags)) {
        if (isAutoManaged(torrent.flags)) {
          return "queued";
        }
        return "paused";
      }
      return "downloading";
    }
    case 4: return "finished";
    case 5: {
      if (isPaused(torrent.flags)) {
        if (isAutoManaged(torrent.flags)) {
          return "seeding_queued";
        }
        return "finished";
      }
      return "seeding";
    }
  }
  return "unknown";
}

type Torrent = {
}

type TorrentsTableProps = {
  isDeleting: (torrent: Torrent) => boolean;
  onMove: (torrent: Torrent) => void;
  onRemove: (torrent: Torrent) => void;
  torrents: Torrent[];
}

export default function TorrentsTable(props: TorrentsTableProps) {
  const {
    isDeleting,
    onMove,
    onRemove,
    torrents
  } = props;

  function torrentKey(torrent: any): string {
    return `${torrent.info_hash[0]},${torrent.info_hash[1]}`;
  }

  return (
    <>
      <Table size={"sm"}>
        <Thead>
          <Tr>
            {/*<Th width={"1px"}><Checkbox /></Th>*/}
            <Th width={"30%"}>Name</Th>
            <Th width={"20px"} textAlign={"right"}>#</Th>
            <Th width={"90px"}>State</Th>
            <Th textAlign={"right"} width={"90px"}>Progress</Th>
            <Th textAlign={"right"} width={"150px"}>Size</Th>
            <Th textAlign={"right"} width={"150px"}>DL</Th>
            <Th textAlign={"right"} width={"150px"}>UL</Th>
            <Th textAlign={"right"} width={"100px"}>Peers</Th>
            <Th textAlign={"right"} width={"100px"}>Seeds</Th>
            <Th width={"1px"}></Th>
          </Tr>
        </Thead>
        <Tbody>
        { torrents.map((t: any) => (
          <Tr key={torrentKey(t)}>
            {/*<Td><Checkbox isChecked={isSelected(t)} onChange={a => setSelected(t, a.target.checked)} /></Td>*/}
            <Td>
              <HStack
                spacing={2}
              >
                <Box>{isDeleting(t) ? <Text color={"gray.600"}>{t.name}</Text> : t.name}</Box>
                <Box>
                  <Tooltip label={t.error.message} shouldWrapChildren>
                    <Icon
                      as={MdWarning}
                      color={"yellow.300"}
                    />
                  </Tooltip>
                </Box>
              </HStack>
            </Td>
            <Td textAlign={"right"}>{t.queue_position < 0 ? "-" : t.queue_position}</Td>
            <Td>
              <Badge colorScheme={stateColor(t)}>{stateString(t)}</Badge>
            </Td>
            <Td textAlign={"right"} whiteSpace={"nowrap"} >{(t.progress * 100).toFixed(t.progress === 1 ? 0 : 1)}%</Td>
            <Td textAlign={"right"} whiteSpace={"nowrap"} >{filesize(t.size).toString()}</Td>
            <Td textAlign={"right"} whiteSpace={"nowrap"} >{t.download_rate > 1024 ? filesize(t.download_rate) + "/s" : "-"}</Td>
            <Td textAlign={"right"} whiteSpace={"nowrap"} >{t.upload_rate > 1024 ? filesize(t.upload_rate) + "/s" : "-"}</Td>
            <Td textAlign={"right"} whiteSpace={"nowrap"} >{t.num_peers > 0 ? t.num_peers : "-"}</Td>
            <Td textAlign={"right"} whiteSpace={"nowrap"} >{t.num_seeds > 0 ? t.num_seeds : "-"}</Td>
            <Td>
              <Menu>
                <MenuButton
                  as={IconButton}
                  icon={<MdOutlineMoreVert />}
                  size={"sm"}
                />
                <MenuList>
                  <MenuGroup title="Actions">
                    <MenuItem
                      icon={<MdFolder />}
                      onClick={() => onMove(t)}
                    >
                      Move
                    </MenuItem>
                    <MenuItem
                      icon={<MdDelete />}
                      onClick={() => onRemove(t)}
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
    </>
  )
}
