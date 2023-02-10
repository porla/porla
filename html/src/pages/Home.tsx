import { Alert, AlertDescription, AlertIcon, AlertTitle, Box, Button, Flex, Grid, GridItem, Heading, HStack, IconButton, Spacer } from "@chakra-ui/react";
import { useEffect, useState } from "react";
import Pager from "../components/Pager";
import RemoveTorrentModal from "../components/RemoveTorrentModal";
import { useInvoker, useRPC } from "../services/jsonrpc";
import AddTorrentModal from "../components/AddTorrentModal";
import MoveTorrentModal from "../components/MoveTorrentModal";
import { MdAddBox } from "react-icons/md";
import { ITorrentsList, PresetsList, Torrent } from "../types";
import TorrentPropertiesModal from "../components/TorrentPropertiesModal";
import TorrentsList from "../components/TorrentsList";
import useTorrentsFilter from "../contexts/TorrentsFilterContext";
import FilterItem from "../components/FilterItem";


export default function Home() {
  const [ page, setPage ] = useState(0);
  const [ moveTorrent, setMoveTorrent ] = useState<Torrent>();
  const [ removeTorrent, setRemoveTorrent ] = useState<{} | null>();
  const [ isDeleting, setIsDeleting ] = useState<Array<string>>([]);
  const [ showAdd, setShowAdd ] = useState(false);
  const [ propsTorrent, setPropsTorrent ] = useState<Torrent | null>();
  const { clearFilter, filters } = useTorrentsFilter();
  const [ order, setOrder ] = useState<any>();

  const { error, data, mutate } = useRPC<ITorrentsList>('torrents.list', () => {
    return {
      filters,
      page,
      order_by: order?.by,
      order_by_dir: order?.dir
    }
  }, {
    refreshInterval: 1000
  });

  const {
    data: presets
  } = useRPC<PresetsList>("presets.list");

  const torrentsMove = useInvoker<void>("torrents.move");
  const torrentsPause = useInvoker<void>("torrents.pause");
  const torrentsRecheck = useInvoker<void>("torrents.recheck");
  const torrentsRemove = useInvoker<void>("torrents.remove");
  const torrentsResume = useInvoker<void>("torrents.resume");

  useEffect(() => {
    setPage(0);
  }, [filters]);

  if (error) {
    return (
      <Alert status="error">
        <AlertIcon />
        <AlertTitle>Could not list torrents.</AlertTitle>
        <AlertDescription flex={1}>
          {
            error.code === -1000 ? (
              <Flex alignItems={"center"} justifyContent={"space-between"}>
                <Box>Query error: { error.message }</Box>
                <Button
                  colorScheme={"red"}
                  size={"xs"}
                  variant={"solid"}
                  onClick={() => clearFilter("query")}
                >
                  Clear query
                </Button>
              </Flex>
            ) : (
              error.toString()
            )
          }
        </AlertDescription>
      </Alert>
    )
  }

  function setDeleting(torrent: any) {
    setIsDeleting(_ => [...isDeleting, torrentKey(torrent)]);
  }

  function torrentKey(torrent: any): string {
    return `${torrent.info_hash[0]},${torrent.info_hash[1]}`;
  }

  if (!data || !presets) {
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
        presets={presets}
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
        data?.torrents_total_unfiltered > 0
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
                    <HStack>
                      {filters && filters.query && (<FilterItem field="query" desc="PQL query" />)}
                    </HStack>
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
                { data.torrents.length > 0
                  ? <TorrentsList
                      orderBy={data.order_by}
                      orderByDir={data.order_by_dir}
                      torrents={data.torrents}
                      isDeleting={() => false}
                      onMove={t => setMoveTorrent(t)}
                      onPause={async (t) => {
                        await torrentsPause({
                          info_hash: t.info_hash
                        });
                        await mutate();
                      }}
                      onRecheck={async (t) => {
                        await torrentsRecheck({ info_hash: t.info_hash });
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
                      onSort={async (by, dir) => {
                        setOrder(() => {
                          return {
                            by,
                            dir
                          }
                        });
                        await mutate();
                      }}
                    />
                  : (
                    <Box p={10} textAlign={"center"}>
                      Filter didn't match any torrents.
                    </Box>
                  )
                }
                
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
