import { Box, Icon, Input, InputGroup, InputLeftElement } from "@chakra-ui/react";
import { useState } from "react";
import { MdFilterAlt } from "react-icons/md";
import useTorrentsFilter from "../contexts/TorrentsFilterContext";

export default function Query() {
  const { setFilter } = useTorrentsFilter();
  const [ query, setQuery ] = useState('');

  return (
    <Box maxW={"400px"} w={"100%"}>
      <InputGroup>
        <InputLeftElement
          pointerEvents={"none"}
          children={<Icon as={MdFilterAlt} />}
        />
        <Input
          fontFamily={"monospace"}
          placeholder={"Filter torrents"}
          onChange={e => setQuery(e.target.value || "")}
          onKeyDown={e => {
            if (e.key === 'Enter') {
              setFilter("query", query);
            }
          }}
        />
      </InputGroup>
    </Box>
  )
}
