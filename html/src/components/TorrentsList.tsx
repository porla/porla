import { Box, Grid, GridItem, StackDivider, Text, VStack } from "@chakra-ui/react";
import { Torrent } from "../types"
import TorrentsListItem from "./TorrentsListItem";

type TorrentsListProps = {
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
        gridTemplateColumns={"32px calc(100% - 220px - 80px) 110px 110px 48px"}
        gridTemplateRows={"0fr"}
        templateAreas={`
          "progress main dl actions"
        `}
      >
        <GridItem></GridItem>
        <GridItem ms={2}><GridTitle title="Name" /></GridItem>
        <GridItem me={2} textAlign={"end"}><GridTitle title="DL" /></GridItem>
        <GridItem me={2} textAlign={"end"}><GridTitle title="UL" /></GridItem>
        <GridItem></GridItem>
      </Grid>
      { props.torrents.map(t => <TorrentsListItem key={torrentKey(t)} torrent={t} />)}
    </VStack>
  );
}
