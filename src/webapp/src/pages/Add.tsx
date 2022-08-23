import React from "react";
import { Formik, Field, Form } from "formik";
import { trpc } from "../utils/trpc";
import { Box, Button, FormControl, FormHelperText, FormLabel, Heading, Input } from "@chakra-ui/react";

function Add() {
  const mutation = trpc.useMutation(["torrents.add"]);

  return (
    <>
      <Box
        backgroundColor={"#fff"}
        border="1px solid #f0f0f0"
        borderRadius={4}
        padding={3}
      >
        <Heading
          color={"#444"}
          marginBottom={3}
          size="md"
        >
          Add torrent
        </Heading>
        <Formik
          initialValues={{ name: "", email: "" }}
          onSubmit={async ({ name }) => {
            await mutation.mutateAsync({ name })
          }}
        >
          <form>
            <FormControl
              marginBottom={3}
            >
              <FormLabel>Magnet link</FormLabel>
              <Field
                as={Input}
                id="path"
                name="path"
                placeholder="magnet:?xt=urn:btih: ..."
                type="text"
              />
              <FormHelperText>The magnet link to add.</FormHelperText>
            </FormControl>

            <FormControl
              marginBottom={3}
            >
              <FormLabel>Save path</FormLabel>
              <Field
                as={Input}
                id="path"
                name="path"
                placeholder="/mnt/downloads"
                type="text"
              />
              <FormHelperText>The full path to where the torrent should be saved.</FormHelperText>
            </FormControl>

            <Button
              colorScheme={"purple"}
              marginTop={3}
              size="sm"
            >
              Add torrent
            </Button>
          </form>
        </Formik>
      </Box>
    </>
  );
}

export default Add;
