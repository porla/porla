import { Alert, AlertDescription, AlertIcon, AlertTitle, Box, Button, Flex, Heading, HStack, IconButton } from "@chakra-ui/react";
import { useState } from "react";
import Pager from "../components/Pager";
import RemoveTorrentModal from "../components/RemoveTorrentModal";
import TorrentsTable from "../components/TorrentsTable";
import { ITorrentsList } from "../services/index";
import { useRPC } from "../services/jsonrpc";
import porla from "../services/index";
import AddTorrentModal from "../components/AddTorrentModal";
import MoveTorrentModal from "../components/MoveTorrentModal";
import { MdAddBox } from "react-icons/md";


export default function Home() {
  const [ page, setPage ] = useState(0);
  const [ moveTorrent, setMoveTorrent ] = useState<{} | null>();
  const [ removeTorrent, setRemoveTorrent ] = useState<{} | null>();
  const [ isDeleting, setIsDeleting ] = useState<Array<string>>([]);
  const [ showAdd, setShowAdd ] = useState(false);

  const { error, data } = useRPC<ITorrentsList>('torrents.list', {
    page
  }, {
    refreshInterval: 1000
  });

  if (error) {
    return (
      <Alert status="error">
        <AlertIcon />
        <AlertTitle>Could not list torrents.</AlertTitle>
        <AlertDescription>{error.toString()}</AlertDescription>
      </Alert>
    )
  }

  function setDeleting(torrent: any) {
    setIsDeleting(_ => [...isDeleting, torrentKey(torrent)]);
  }

  function torrentKey(torrent: any): string {
    return `${torrent.info_hash[0]},${torrent.info_hash[1]}`;
  }

  if (!data) {
    return (
      <Box p={10} textAlign={"center"}>Loading...</Box>
    )
  }

  return (
    <>
      <AddTorrentModal
        isOpen={showAdd}
        onClose={() => { setShowAdd(false); }}
      />

      <MoveTorrentModal
        torrent={moveTorrent}
        onMove={async (torrent, path) => {
          if (torrent === null) {
            return setMoveTorrent(null);
          }

          if (!path || path === null || path === "") {
            alert('no');
            return;
          }

          await porla.torrents.move(torrent.info_hash, path);

          setMoveTorrent(null);
        }}
      />

      <RemoveTorrentModal
        torrent={removeTorrent}
        onRemove={async (torrent, remove_data) => {
          if (torrent === null) {
            return setRemoveTorrent(null);
          }
          setDeleting(torrent);
          await porla.torrents.remove(torrent.info_hash, remove_data);
          setRemoveTorrent(null);
        }}
      />

      {
        data?.torrents.length > 0
          ? (
            <>
              <Flex
                justifyContent={"space-between"}
                m={3}
              >
                <HStack spacing={0}>
                  <Heading as="h3" size={"md"}>Torrents</Heading>
                  <IconButton
                    aria-label={"Add torrent"}
                    colorScheme={"purple"}
                    icon={<MdAddBox />}
                    size={"lg"}
                    variant={"link"}
                    onClick={() => setShowAdd(true)}
                  />
                </HStack>
                <Pager
                  page={page}
                  pageSize={data?.page_size || 0}
                  totalItems={data?.torrents_total || 0}
                  setPage={(p) => {
                    setPage(p);
                  }}
                />
              </Flex>
              <TorrentsTable
                isDeleting={() => false}
                onMove={t => setMoveTorrent(t)}
                onRemove={(torrent) => setRemoveTorrent(torrent)}
                torrents={data?.torrents || []}
              />
            </>
          ) : (
            <Box p={10} textAlign={"center"}>
              <Box>
                No torrents added.
              </Box>
              <Button
                mt={5}
                colorScheme={"purple"}
                variant={"outline"}
                onClick={() => setShowAdd(true)}
              >
                Add torrent
              </Button>
            </Box>
          )
      }
    </>
  )
}
