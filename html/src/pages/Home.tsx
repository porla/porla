import { Alert, AlertDescription, AlertIcon, AlertTitle, Box, Button, Flex, Grid, GridItem, Heading, HStack, IconButton, Spacer } from "@chakra-ui/react";
import { useState } from "react";
import Pager from "../components/Pager";
import RemoveTorrentModal from "../components/RemoveTorrentModal";
import TorrentsTable from "../components/TorrentsTable";
import { useInvoker, useRPC } from "../services/jsonrpc";
import AddTorrentModal from "../components/AddTorrentModal";
import MoveTorrentModal from "../components/MoveTorrentModal";
import { MdAddBox } from "react-icons/md";
import { ITorrentsList, Torrent } from "../types";
import TorrentPropertiesModal from "../components/TorrentPropertiesModal";
import TorrentsList from "../components/TorrentsList";


export default function Home() {
  const [ page, setPage ] = useState(0);
  const [ moveTorrent, setMoveTorrent ] = useState<Torrent>();
  const [ removeTorrent, setRemoveTorrent ] = useState<{} | null>();
  const [ isDeleting, setIsDeleting ] = useState<Array<string>>([]);
  const [ showAdd, setShowAdd ] = useState(false);
  const [ propsTorrent, setPropsTorrent ] = useState<Torrent | null>();

  const { error, data, mutate } = useRPC<ITorrentsList>('torrents.list', {
    page
  }, {
    refreshInterval: 1000
  });

  const torrentsMove = useInvoker<void>("torrents.move");
  const torrentsPause = useInvoker<void>("torrents.pause");
  const torrentsRemove = useInvoker<void>("torrents.remove");
  const torrentsResume = useInvoker<void>("torrents.resume");

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
    <Box height={"100%"}>
      <AddTorrentModal
        isOpen={showAdd}
        onClose={async () => {
          setShowAdd(false);
          await mutate();
        }}
      />

      <MoveTorrentModal
        torrent={moveTorrent}
        onMove={async (torrent, path) => {
          if (torrent === null) {
            return setMoveTorrent(undefined);
          }

          if (!path || path === null || path === "") {
            alert('no');
            return;
          }

          await torrentsMove({
            info_hash: torrent.info_hash,
            path
          });

          setMoveTorrent(undefined);

          await mutate();
        }}
      />

      <RemoveTorrentModal
        torrent={removeTorrent}
        onRemove={async (torrent, remove_data) => {
          if (torrent === null) {
            return setRemoveTorrent(null);
          }

          setDeleting(torrent);

          await torrentsRemove({
            info_hashes: [torrent.info_hash],
            remove_data
          });

          setRemoveTorrent(null);
          await mutate();
        }}
      />

      <TorrentPropertiesModal
        torrent={propsTorrent}
        onClose={() => setPropsTorrent(null)}
      />

      {
        data?.torrents.length > 0
          ? (
            <Grid
              height={"100%"}
              templateAreas={`
                "title"
                "content"
              `}
              gridTemplateColumns={"1fr"}
              gridTemplateRows={"min-content fit-content(100%)"}
            >
              <GridItem area={"title"}>
                <HStack
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
                  <Spacer />
                  <Pager
                    page={page}
                    pageSize={data?.page_size || 0}
                    totalItems={data?.torrents_total || 0}
                    setPage={(p) => {
                      setPage(p);
                    }}
                  />
                </HStack>
              </GridItem>

              <GridItem area={"content"} overflow={"scroll"}>
                <TorrentsList
                  torrents={data.torrents}
                  isDeleting={() => false}
                  onMove={t => setMoveTorrent(t)}
                  onPause={async (t) => {
                    await torrentsPause({
                      info_hash: t.info_hash
                    });
                    await mutate();
                  }}
                  onRemove={(torrent) => setRemoveTorrent(torrent)}
                  onResume={async (t) => {
                    await torrentsResume({
                      info_hash: t.info_hash
                    });
                    await mutate();
                  }}
                  onShowProperties={t => setPropsTorrent(t)}
                />
              </GridItem>
            </Grid>
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
    </Box>
  )
}
