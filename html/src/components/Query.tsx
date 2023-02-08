import { Box, Icon, Input, InputGroup, InputLeftElement } from "@chakra-ui/react";
import { MdFilterAlt } from "react-icons/md";

export default function Query() {
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
        />
      </InputGroup>
    </Box>
  )
}