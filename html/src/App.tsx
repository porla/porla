import { Box, Flex, IconButton, Image, Link, Stack } from '@chakra-ui/react';
import { MdSettings } from 'react-icons/md';
import { NavLink, Outlet } from 'react-router-dom';

import Isotype from './assets/isotype.svg';

function App() {
  return (
    <Box className="App">
      <Flex
        borderBottomColor={"whiteAlpha.400"}
        borderBottomWidth={"1px"}
        boxShadow={"sm"}
        justifyContent={"space-between"}
        mb={2}
        p={3}
      >
        <Stack direction={"row"} alignItems={"center"}>
          <Image src={Isotype} width={"32px"} />
          <Link
          >
            Torrents
          </Link>
        </Stack>

        <IconButton
          aria-label=''
          as={NavLink}
          icon={<MdSettings />}
          to={"/settings"}
          variant={"link"}
        />
      </Flex>
      <Outlet />
    </Box>
  )
}

export default App
