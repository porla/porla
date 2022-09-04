import { Button, Checkbox, FormControl, FormHelperText, FormLabel, Input, Select } from "@chakra-ui/react";
import { Field, Formik } from "formik";
import React from "react";
import { trpc } from "../utils/trpc";

import Card from "../components/Card";
import Loading from "../components/Loading";

export default function Settings() {
  const config = trpc.useQuery(["config.get", [
    "default_save_path",
    "proxy_type",
    "proxy_hostname",
    "proxy_port"
  ]]);

  const setConfig = trpc.useMutation(["config.set"]);

  if (!config.data) {
    return <Loading />
  }

  return (
    <>
      <Formik
        initialValues={{
          default_save_path: config.data.default_save_path || "",
          proxy_type: config.data.proxy_type || 0,
          proxy_hostname: config.data.proxy_hostname || "",
          proxy_port: config.data.proxy_port || 1080
        }}
        onSubmit={async (values) => {
          await setConfig.mutateAsync({
            ...values,
            // force number for certain properties
            proxy_type: parseInt(values.proxy_type)
          });
        }}
      >
        {({ handleChange, handleSubmit, values }) => (
          <form onSubmit={handleSubmit}>
            <Card heading={"General"}>
              <FormControl
                marginBottom={3}
              >
                <FormLabel htmlFor="default_save_path">Default save path</FormLabel>
                <Field
                  as={Input}
                  id="default_save_path"
                  name="default_save_path"
                  placeholder="/mnt/downloads"
                  type="text"
                />
                <FormHelperText>The default save path. This will be used when no other save path has been specified.</FormHelperText>
              </FormControl>
            </Card>

            <Card heading="Proxy" mt={5}>
              <FormControl
                marginBottom={3}
              >
                <FormLabel>Type</FormLabel>
                <Select
                  id="proxy_type"
                  name="proxy_type"
                  onChange={handleChange}
                  value={values.proxy_type}
                >
                  <option value="0">None</option>
                  <option value="5">SOCKS5</option>
                </Select>
              </FormControl>
              <FormControl
                marginBottom={3}
              >
                <FormLabel>Host</FormLabel>
                <Field
                  as={Input}
                  id="proxy_hostname"
                  isDisabled={values.proxy_type==0}
                  name="proxy_hostname"
                  placeholder="10.64.0.1"
                  type="text"
                />
                <FormHelperText>The proxy host.</FormHelperText>
              </FormControl>
              <FormControl
                marginBottom={3}
              >
                <FormLabel>Port</FormLabel>
                <Field
                  as={Input}
                  id="proxy_port"
                  isDisabled={values.proxy_type==0}
                  name="proxy_port"
                  placeholder="1080"
                  type="number"
                />
                <FormHelperText>The proxy port.</FormHelperText>
              </FormControl>
            </Card>

            <Button
              colorScheme={"purple"}
              marginTop={5}
              type="submit"
            >
              Save settings
            </Button>
          </form>
        )}
      </Formik>
    </>
  );
}
