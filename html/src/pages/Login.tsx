import { Button } from "@chakra-ui/button";
import { FormControl, FormLabel } from "@chakra-ui/form-control";
import { Input } from "@chakra-ui/input";
import { Box, Flex, Text } from "@chakra-ui/layout";
import { Heading, Spinner, VStack } from "@chakra-ui/react";
import { Field, Form, Formik } from "formik";
import { useEffect, useState } from "react";
import { Navigate, useNavigate } from "react-router-dom";
import prefixPath from "../base";
import useAuth from "../contexts/auth";

type Status = {
  level: "error" | "info",
  message: string;
}

export default function Login() {
  const { login } = useAuth();

  const navigate = useNavigate();
  const [ status, setStatus ] = useState<Status | undefined>();
  const [ systemStatus, setSystemStatus ] = useState<string | undefined>();

  useEffect(() => {
    fetch(prefixPath("/api/v1/system"))
      .then(r => r.json())
      .then(s => {
        setSystemStatus(s.status);
      });
  }, []);

  if (systemStatus === "setup") {
    return <Navigate to={"/setup"} />
  }

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
        <Box
          mb={5}
        >
          <Heading size={"lg"}>Login to Porla</Heading>
        </Box>

        <Formik
          initialValues={{
            username: "",
            password: ""
          }}
          onSubmit={async (values) => {
            setStatus({ level: "info", message: "Signing in" });

            try {
              await login(values.username, values.password);
              return navigate("/");
            }
            catch (err) {
              setStatus({ level: "error", message: "Invalid username or password" });
            }

            setTimeout(() => setStatus(undefined), 3000);
          }}
        >
          {({ isSubmitting }) => (
            <Form>
              <VStack spacing={5}>
                <Field name="username">
                  {(w: any) => (
                    <FormControl isDisabled={isSubmitting}>
                      <FormLabel>Username</FormLabel>
                      <Input {...w.field} />
                    </FormControl>
                  )}
                </Field>

                <Field name="password">
                  {(w: any) => (
                    <FormControl isDisabled={isSubmitting} mt={3}>
                      <FormLabel>Password</FormLabel>
                      <Input {...w.field} type={"password"} />
                    </FormControl>
                  )}
                </Field>
              </VStack>

              <Flex
                alignItems={"center"}
                justifyContent={"space-between"}
                mt={5}
              >
                <Flex alignItems={"center"} visibility={!!status ? "visible" : "hidden"}>
                  {
                    status?.level === "info"
                      ? <Spinner me={2} size={"sm"} />
                      : <></>
                  }
                  <Text fontSize={"sm"}>{status?.message}</Text>
                </Flex>

                <Flex alignItems={"center"}>
                  <Button
                    colorScheme={"purple"}
                    disabled={isSubmitting}
                    type={"submit"}
                  >
                      Login
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
