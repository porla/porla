import { Button, FormControl, FormHelperText, FormLabel, Input } from "@chakra-ui/react";
import { Field, Formik } from "formik";
import React from "react";
import Card from "../components/Card";

export default function Settings() {
  return (
    <Card heading={"General"}>
      <Formik
          initialValues={{ name: "", email: "" }}
          onSubmit={async ({ name }) => {
          }}
      >
        <form>
          <FormControl
            marginBottom={3}
          >
            <FormLabel>Default save path</FormLabel>
            <Field
              as={Input}
              id="path"
              name="path"
              placeholder="/mnt/downloads"
              type="text"
            />
            <FormHelperText>The default save path. This will be used when no other save path has been specified.</FormHelperText>
          </FormControl>

          <Button
            colorScheme={"purple"}
            marginTop={3}
            size="sm"
          >
            Save settings
          </Button>
        </form>
      </Formik>
    </Card>
  );
}
