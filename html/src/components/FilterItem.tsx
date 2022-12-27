import { Tag, TagCloseButton, TagLabel, TagLeftIcon } from "@chakra-ui/react";
import { MdFilter, MdFolderOpen, MdLabel, MdTag } from "react-icons/md";
import useTorrentsFilter from "../contexts/TorrentsFilterContext";
import { Filter } from "../types";

type FilterItemProps = {
  filter: Filter;
}

function FilterFieldIcon(field: string) {
  switch (field) {
    case "category":  return MdLabel;
    case "save_path": return MdFolderOpen;
    case "tags":      return MdTag;
  }
  return MdFilter;
}

export default function FilterItem(props: FilterItemProps) {
  const { clearFilter } = useTorrentsFilter();

  return (
    <Tag
      size={"sm"}
    >
      <TagLeftIcon as={FilterFieldIcon(props.filter.field)} />
      <TagLabel>{props.filter.args}</TagLabel>
      <TagCloseButton onClick={() => clearFilter(props.filter.field)} />
    </Tag>
  )
}
