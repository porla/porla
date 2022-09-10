import React from "react";
import { Icon } from "@chakra-ui/react";
import { MdClose, MdDone, MdDownload, MdOutlineWarningAmber, MdPause, MdUpload } from "react-icons/md";
import { getState, getStateColor } from "../utils/torrentStates";

interface IProgressLabelProps {
  t: Function;
  torrent: any;
  showNumber: boolean;
}

function ProgressLabel(props: IProgressLabelProps) {
  const { t, torrent, showNumber } = props;
  const state: string = getState(torrent);
  const color: string = getStateColor(state);

  const icons = {
    "completed": MdDone,
    "downloading": MdDownload,
    "error": MdClose,
    "paused": MdPause,
    "seeding": MdUpload,
    "unknown": MdOutlineWarningAmber,
  };

  return showNumber && state === "downloading"
    ? (<>{Math.trunc(torrent.progress * 100)}%</>)
    : (<Icon className="icon-state" aria-label={t(state)} as={icons[state]} color={color} w={5} h={5}/>)
}

export default ProgressLabel;