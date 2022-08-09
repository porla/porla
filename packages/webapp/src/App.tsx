import {
  ChakraProvider,
  Box,
  Grid,
  theme,
} from "@chakra-ui/react"

export const App = () => (
  <ChakraProvider theme={theme}>
    <Box textAlign="center" fontSize="xl">
      <Grid minH="100vh" p={3}>
        Porla
      </Grid>
    </Box>
  </ChakraProvider>
)
