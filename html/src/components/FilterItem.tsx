import { Tag, TagCloseButton, TagLabel, TagLeftIcon } from "@chakra-ui/react";
import { MdFilterAlt, MdFolderOpen, MdLabel, MdTag } from "react-icons/md";
import useTorrentsFilter from "../contexts/TorrentsFilterContext";
import { TorrentsListFilters } from "../types";

type FilterItemProps = {
  field: keyof TorrentsListFilters;
  desc: string;
}

function FilterFieldIcon(field: keyof TorrentsListFilters) {
  switch (field) {
    case "category":  return MdLabel;
    case "query":     return MdFilterAlt;
    case "save_path": return MdFolderOpen;
    case "tags":      return MdTag;
  }
}

export default function FilterItem(props: FilterItemProps) {
  const { clearFilter } = useTorrentsFilter();

  return (
    <Tag
      size={"sm"}
    >
      <TagLeftIcon as={FilterFieldIcon(props.field)} />
      <TagLabel>{props.desc}</TagLabel>
      <TagCloseButton onClick={() => clearFilter(props.field)} />
    </Tag>
  )
}
