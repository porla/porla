import { Button } from "@chakra-ui/button";
import { FormControl, FormHelperText, FormLabel } from "@chakra-ui/form-control";
import { Input } from "@chakra-ui/input";
import { Box, Flex, Text } from "@chakra-ui/layout";
import { Heading, Image, Spinner } from "@chakra-ui/react";
import { Field, Form, Formik } from "formik";
import { useState } from "react";
import { useNavigate } from "react-router-dom";
import Isotype from '../assets/isotype.svg';
import useAuth from "../contexts/auth";
import auth from "../services/auth";

type Status = {
  level: "error" | "info",
  message: string;
}

export default function Setup() {
  const { login } = useAuth();
  const navigate = useNavigate();
  const [ status, setStatus ] = useState<Status | undefined>();

  return (
    <Flex
      justifyContent={"center"}
    >
      <Box
        maxW={"400px"}
        mt={5}
        p={5}
        w={"100%"}
      >
        <Flex
          justifyContent={"center"}
          mb={5}
        >
          <Image src={Isotype} width={"92px"} />
        </Flex>
        <Box
        >
          <Heading size={"lg"}>Welcome to Porla</Heading>
          <Text my={5}>Please set up Porla by creating your user account. This account is used to access Porla.</Text>
        </Box>

        <Formik
          initialValues={{
            username: "",
            password: ""
          }}
          onSubmit={async ({ username, password }) => {
            setStatus({ level: "info", message: "Creating user account" });

            const { ok } = await auth.init(username, password);

            if (!ok) {
              return setStatus({ level: "error", message: "Failed to create user account" });
            }

            setStatus({ level: "info", message: "Signing in" });

            await login(username, password);

            navigate("/");
          }}
        >
          {({ isSubmitting }) => (
            <Form>
              <Field name="username">
                {(w: any) => (
                  <FormControl isDisabled={isSubmitting}>
                    <FormLabel>Username</FormLabel>
                    <Input {...w.field} />
                    <FormHelperText>Your chosen username</FormHelperText>
                  </FormControl>
                )}
              </Field>

              <Field name="password">
                {(w: any) => (
                  <FormControl isDisabled={isSubmitting} mt={3}>
                    <FormLabel>Password</FormLabel>
                    <Input {...w.field} type={"password"} />
                    <FormHelperText>A secure password</FormHelperText>
                  </FormControl>
                )}
              </Field>

              <FormControl isDisabled={isSubmitting} mt={3}>
                <FormLabel>Repeat password</FormLabel>
                <Input type={"password"} />
                <FormHelperText>Repeat your password</FormHelperText>
              </FormControl>

              <Flex
                alignItems={"center"}
                justifyContent={"space-between"}
                mt={5}
              >
                <Flex alignItems={"center"} visibility={!!status ? "visible" : "hidden"}>
                  <Spinner me={2} size={"sm"} />
                  <Text fontSize={"sm"}>{status?.message}</Text>
                </Flex>

                <Flex alignItems={"center"}>
                  <Button
                    colorScheme={"purple"}
                    disabled={isSubmitting}
                    type={"submit"}
                  >
                      Create account
                  </Button>
                </Flex>
              </Flex>
            </Form>
          )}
        </Formik>
      </Box>
    </Flex>
  )
}
