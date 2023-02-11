import { Box, Icon, Input, InputGroup, InputLeftElement } from "@chakra-ui/react";
import { useEffect, useState } from "react";
import { MdFilterAlt } from "react-icons/md";
import useTorrentsFilter from "../contexts/TorrentsFilterContext";

export default function Query() {
  const { filters, setFilter } = useTorrentsFilter();
  const [ query, setQuery ] = useState('');

  useEffect(() => {
    if (filters.query === null || filters.query === undefined || filters.query === "") {
      setQuery('');
    }
  }, [filters])

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
          value={query}
        />
      </InputGroup>
    </Box>
  )
}
