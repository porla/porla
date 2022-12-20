import { Grid, GridItem, StackDivider, Text, VStack } from "@chakra-ui/react";
import { Torrent } from "../types"
import TorrentsListItem from "./TorrentsListItem";

type TorrentsListProps = {
  isDeleting: (torrent: Torrent) => boolean;
  onMove: (torrent: Torrent) => void;
  onPause: (torrent: Torrent) => void;
  onRemove: (torrent: Torrent) => void;
  onResume: (torrent: Torrent) => void;
  onShowProperties: (torrent: Torrent) => void;
  torrents: Torrent[];
}

type GridTitleProps = {
  title: string;
}

function torrentKey(torrent: Torrent): string {
  return `${torrent.info_hash[0]},${torrent.info_hash[1]}`;
}

function GridTitle(props: GridTitleProps) {
  return (
    <Text
      color={"gray.400"}
      fontFamily={"heading"}
      fontSize={"xs"}
      fontWeight={"bold"}
      letterSpacing={"wider"}
      textTransform={"uppercase"}
    >
      {props.title}
    </Text>
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
        gridTemplateColumns={"32px 24px 1fr 110px 110px 100px 48px"}
        gridTemplateRows={"0fr"}
      >
        <GridItem></GridItem>
        <GridItem alignSelf={"center"} ms={2} textAlign={"center"}><GridTitle title="#" /></GridItem>
        <GridItem ms={2}><GridTitle title="Name" /></GridItem>
        <GridItem me={2} textAlign={"end"}><GridTitle title="Ratio" /></GridItem>
        <GridItem me={2} textAlign={"end"}><GridTitle title="DL" /></GridItem>
        <GridItem me={2} textAlign={"end"}><GridTitle title="UL" /></GridItem>
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
          onRemove={props.onRemove}
          onResume={props.onResume}
          onShowProperties={props.onShowProperties} />
      ))}
    </VStack>
  );
}
