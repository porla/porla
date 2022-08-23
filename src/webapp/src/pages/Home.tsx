import { Box, Flex, IconButton, Input, Table, Th, Thead, Tr } from "@chakra-ui/react";
import React from "react";
import { VscMenu } from "react-icons/vsc";
import { trpc } from "../utils/trpc";

function Home() {
  const torrents = trpc.useQuery(["torrents.list"]);

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
          icon={<VscMenu />}
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
              <Th>Name</Th>
              <Th>Size</Th>
              <Th>Peers</Th>
            </Tr>
          </Thead>
        </Table>
      </Box>
    </>
  );
}

export default Home;
