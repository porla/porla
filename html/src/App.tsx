import { Box, Flex, Image } from '@chakra-ui/react';
import { Outlet } from 'react-router-dom';

import Isotype from './assets/isotype.svg';

function App() {
  return (
    <Box className="App">
      <Flex
        borderBottomColor={"whiteAlpha.400"}
        borderBottomWidth={"1px"}
        boxShadow={"sm"}
        mb={2}
        p={3}
      >
        <Image src={Isotype} width={"32px"} />
      </Flex>
      <Outlet />
    </Box>
  )
}

export default App
