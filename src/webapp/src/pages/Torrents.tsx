import { Box, Flex, Icon, IconButton, Input, Table, Tbody, Td, Th, Thead, Tr } from "@chakra-ui/react";
import React from "react";
import { VscCircleLargeFilled, VscCloudDownload, VscCloudUpload, VscFolder, VscMenu } from "react-icons/vsc";
import { trpc } from "../utils/trpc";
import filesize from "filesize";

function Torrents() {
  const torrents = trpc.useQuery(["torrents.list"], {
    refetchInterval: 1000
  });

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
              <Th w={"16px"}></Th>
              <Th>Name</Th>
              <Th>Size</Th>
              <Th>DL</Th>
              <Th>UL</Th>
              <Th>Peers</Th>
              <Th w={"16px"}></Th>
            </Tr>
          </Thead>
          <Tbody>
            { torrents.data.map((t,idx) => (
              <Tr key={idx}>
                <Td
                  paddingEnd={0}
                >
                  <Icon
                    as={VscCircleLargeFilled}
                    color={"green.400"}
                    w={"16px"}
                  />
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
                      as={VscFolder}
                      mr={1}
                    />
                    {t.save_path}
                  </Flex>
                </Td>
                <Td>{filesize(t.size)}</Td>
                <Td>{t.download_rate < 1024 ? "-" : t.download_rate}</Td>
                <Td>{t.upload_rate   < 1024 ? "-" : t.upload_rate}</Td>
                <Td></Td>
                <Td
                  paddingEnd={0}
                >
                  <IconButton
                    aria-label=""
                    icon={<VscMenu />}
                    size={"xs"}
                  />
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
