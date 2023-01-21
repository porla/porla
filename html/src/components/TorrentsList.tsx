import { Box, Flex, Grid, GridItem, Icon, Link, StackDivider, Text, VStack } from "@chakra-ui/react";
import { MdKeyboardArrowDown, MdKeyboardArrowUp } from "react-icons/md";
import { Torrent } from "../types"
import TorrentsListItem from "./TorrentsListItem";

type TorrentsListProps = {
  isDeleting: (torrent: Torrent) => boolean;
  onMove: (torrent: Torrent) => void;
  onPause: (torrent: Torrent) => void;
  onRecheck: (torrent: Torrent) => void;
  onRemove: (torrent: Torrent) => void;
  onResume: (torrent: Torrent) => void;
  onShowProperties: (torrent: Torrent) => void;
  onSort: (orderBy: string, orderByDir: string) => void;
  orderBy: string;
  orderByDir: string;
  torrents: Torrent[];
}

type GridTitleProps = {
  field: string;
  onSort: (orderBy: string, orderByDir: string) => void;
  orderBy: string;
  orderByDir: string;
  title: string;
}

function torrentKey(torrent: Torrent): string {
  return `${torrent.info_hash[0]},${torrent.info_hash[1]}`;
}

function GridTitle(props: GridTitleProps) {
  return (
    <Flex
      alignItems={"center"}
      color={"gray.400"}
      fontFamily={"heading"}
      fontWeight={"bold"}
      letterSpacing={"wider"}
      textTransform={"uppercase"}
    >
      <Text
        as={Link}
        fontSize={"xs"}
        onClick={() => {
          props.onSort(
            props.field,
            props.field === props.orderBy
              ? props.orderByDir === "asc" ? "desc" : "asc"
              : props.orderByDir)
        }}
      >
        {props.title}
      </Text>
      {props.field === props.orderBy && (
        <Icon
          as={
            props.orderByDir === "asc"
              ? MdKeyboardArrowUp
              : MdKeyboardArrowDown
          }
        />
      )}
    </Flex>
  )
}

export default function TorrentsList(props: TorrentsListProps) {
  return (
    <VStack
      align={"stretch"}
      divider={<StackDivider borderColor='whiteAlpha.200' />}
      mx={2}
      spacing={1}
    >
      <Grid
        gridTemplateColumns={"32px 48px 1fr 100px 110px 110px 100px 48px"}
        gridTemplateRows={"0fr"}
      >
        <GridItem></GridItem>
        <GridItem alignSelf={"center"} display={"flex"} ms={2} justifyContent={"center"}>
          <GridTitle field="queue_position" onSort={props.onSort} orderBy={props.orderBy} orderByDir={props.orderByDir} title="#" />
        </GridItem>
        <GridItem ms={2}>
          <GridTitle field="name" onSort={props.onSort} orderBy={props.orderBy} orderByDir={props.orderByDir} title="Name" />
        </GridItem>
        <GridItem display={"flex"} me={2} justifyContent={"end"}>
          <GridTitle field={"eta"} onSort={props.onSort} orderBy={props.orderBy} orderByDir={props.orderByDir} title="ETA" />
        </GridItem>
        <GridItem display={"flex"} me={2} justifyContent={"end"}>
          <GridTitle field={"ratio"} onSort={props.onSort} orderBy={props.orderBy} orderByDir={props.orderByDir} title="Ratio" />
        </GridItem>
        <GridItem display={"flex"} me={2} justifyContent={"end"}>
          <GridTitle field={"download_rate"} onSort={props.onSort} orderBy={props.orderBy} orderByDir={props.orderByDir} title="DL" />
        </GridItem>
        <GridItem display={"flex"} me={2} justifyContent={"end"}>
          <GridTitle field={"upload_rate"} onSort={props.onSort} orderBy={props.orderBy} orderByDir={props.orderByDir} title="UL" />
        </GridItem>
        <GridItem></GridItem>
      </Grid>
      { props.torrents.map((t, idx) => (
        <TorrentsListItem
          key={torrentKey(t)}
          index={idx}
          torrent={t}
          onMove={props.onMove}
          isDeleting={props.isDeleting}
          onPause={props.onPause}
          onRecheck={props.onRecheck}
          onRemove={props.onRemove}
          onResume={props.onResume}
          onShowProperties={props.onShowProperties} />
      ))}
    </VStack>
  );
}
