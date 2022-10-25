import { Badge, Box, Button, Flex, IconButton, Image, Menu, MenuButton, MenuGroup, MenuItem, MenuList, Table, Tbody, Td, Th, Thead, Tr } from '@chakra-ui/react';
import { MdDelete, MdOutlineMoreVert, MdPause, MdPlayArrow } from 'react-icons/md';
import { filesize } from 'filesize';
import useSWR from 'swr';

import Isotype from './assets/isotype.svg';
import Pager from './components/Pager';
import { useEffect, useState } from 'react';

const fetcher = async (input: string, init: RequestInit, ...args: any[]) => {
  const res = await fetch('/api/v1/jsonrpc', {
    body: JSON.stringify({
      jsonrpc: '2.0',
      method: input,
      params: {}
    }),
    method: 'POST'
  });

  return res.json();
};

function checkBit(flags: number, bit: number) {
  return (flags & (1<<bit)) === 1<<bit;
}

function isAutoManaged(flags: number) {
  return checkBit(flags, 5);
}

function isPaused(flags: number) {
  return (flags & (1<<4)) === 1<<4;
}

function stateColor(state: number) {
  switch (state) {
    case 3: return "blue";
    case 5: return "green";
  }
  return "default";
}

function stateString(torrent: any) {
  switch (torrent.state) {
    case 1: return "checking_files";
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
    case 5: return "seeding";
  }
  return "unknown";
}

function App() {
  const [ page, setPage ] = useState(0);
  const { error, data } = useSWR(['torrents.list'], fetcher, { refreshInterval: 1000 });

  if (error) {
    return (
      <div>{error.toString()}</div>
    )
  }

  if (!data) {
    return (<div>Loading...</div>)
  }

  return (
    <Box className="App">
      <Flex
        borderBottomColor={"whiteAlpha.400"}
        borderBottomWidth={"1px"}
        boxShadow={"sm"}
        mb={2}
        p={3}
      >
        <Image src={Isotype} width={"32px"} />
      </Flex>
      <Flex
        justifyContent={"end"}
        m={3}
      >
        <Pager
          page={page}
          pageSize={data.result.page_size}
          totalItems={data.result.torrents_total}
          setPage={(p) => {
            setPage(p);
          }}
        />
      </Flex>
      <Table size={"sm"}>
        <Thead>
          <Tr>
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
        { data.result.torrents.map((t: any) => (
          <Tr>
            <Td>{t.name}</Td>
            <Td textAlign={"right"}>{t.queue_position < 0 ? "-" : t.queue_position}</Td>
            <Td>
              <Badge colorScheme={stateColor(t.state)}>{stateString(t)}</Badge>
            </Td>
            <Td textAlign={"right"}>{(t.progress * 100).toFixed(t.progress === 1 ? 0 : 1)}%</Td>
            <Td textAlign={"right"}>{filesize(t.size).toString()}</Td>
            <Td textAlign={"right"}>{t.download_rate > 1024 ? filesize(t.download_rate) + "/s" : "-"}</Td>
            <Td textAlign={"right"}>{t.upload_rate > 1024 ? filesize(t.upload_rate) + "/s" : "-"}</Td>
            <Td textAlign={"right"}>{t.num_peers}</Td>
            <Td textAlign={"right"}>{t.num_seeds}</Td>
            <Td>
              <Menu>
                <MenuButton
                  as={IconButton}
                  icon={<MdOutlineMoreVert />}
                  size={"sm"}
                />
                <MenuList>
                  <MenuGroup title="Actions">
                    {isPaused(t.flags)
                      ? <MenuItem
                          icon={<MdPlayArrow />}
                        >
                          Resume
                        </MenuItem>

                      : <MenuItem
                          icon={<MdPause />}
                        >
                          Pause
                        </MenuItem>
                    }
                    <MenuItem
                      icon={<MdDelete />}
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
  )
}

export default App
