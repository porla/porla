import { Button } from "@chakra-ui/button";
import { FormControl, FormHelperText, FormLabel } from "@chakra-ui/form-control";
import { Input } from "@chakra-ui/input";
import { Box, Flex } from "@chakra-ui/layout";
import { Formik } from "formik";

export default function Setup() {
  return (
    <Flex
      justifyContent={"center"}
    >
      <Box
        border={"1px"}
        borderColor={"whiteAlpha.200"}
        maxW={"400px"}
        mt={5}
        p={5}
        w={"100%"}
      >
        <Formik
          initialValues={{
            username: "",
            password: ""
          }}
          onSubmit={async () => {

          }}
        >
          {({ }) => (
            <>
              <FormControl>
                <FormLabel>Username</FormLabel>
                <Input />
                <FormHelperText>Your chosen username</FormHelperText>
              </FormControl>
              <FormControl mt={3}>
                <FormLabel>Password</FormLabel>
                <Input type={"password"} />
                <FormHelperText>A secure password</FormHelperText>
              </FormControl>
              <Flex
                justifyContent={"end"}
              >
                <Button
                    colorScheme={"purple"}
                    mt={3}
                >
                    Create account
                </Button>
              </Flex>
            </>
          )}
        </Formik>
      </Box>
    </Flex>
  )
}
