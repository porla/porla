import { Box, Button, Drawer, DrawerBody, DrawerCloseButton, DrawerContent, DrawerFooter, DrawerHeader, DrawerOverlay, propNames, Spinner, Tab, TabList, TabPanel, TabPanels, Tabs } from "@chakra-ui/react";
import { Form, Formik } from "formik";
import { useState } from "react";
import * as Yup from "yup";

import { ISettingsDict, ISettingsList } from "../services";
import { jsonrpc, useRPC } from "../services/jsonrpc";
import GeneralSettingsTab from "./settings/GeneralSettingsTab";
import ProxySettingsTab from "./settings/ProxySettingsTab";

type SettingsDrawerProps = {
  isOpen: boolean;
  onClose: () => void;
}

const SettingsSchema = Yup.object().shape({
  listen_interfaces: Yup.array()
    .transform(function (value, originalValue) {
      if (this.isType(value) && value !== null) {
        return value;
      }
      return originalValue ? originalValue.split(/[\s,]+/) : [];
    })
    .required("Listen interfaces is required")
    .of(
      Yup.string()
        .matches(/^(.*)\:[\d]+$/gm, { message: "Listen interfaces contains invalid data." })),
  proxy_type: Yup.number().required(),
  proxy_hostname: Yup.string()
    .when("proxy_type", {
      is: (v: number) => v > 0,
      then: (schema) => schema.required("Proxy hostname is required.")
    }),
  proxy_port: Yup.number()
    .when("proxy_type", {
      is: (v: number) => v > 0,
      then: (schema) => schema
        .required("Port number is required")
        .moreThan(0, "Port number must be greater than 0.")
    })
});

type SettingsFormProps = {
  onSubmitting: (isSubmitting: boolean) => void;
  settings: ISettingsDict;
}

function SettingsForm(props: SettingsFormProps) {
  const { onSubmitting, settings } = props;

  return (
    <Formik
      initialValues={{
        ...settings
      }}
      onSubmit={async (values) => {
        onSubmitting(true);

        const delta = Object.entries(values)
          .reduce((prev, [key, value]) => {
            if ((settings as any)[key] !== value) {
              prev[key] = value;
            }
            return prev;
          }, {} as {
            [index: string]: boolean | number | string
          });

        if ("listen_interfaces" in delta && typeof delta.listen_interfaces === "string") {
          delta.listen_interfaces = delta.listen_interfaces.replace("\n", ",");
        }

        await jsonrpc("session.settings.update", {
          settings: delta
        });

        // Not even ashamed of this.
        await new Promise(r => setTimeout(r, 500));

        onSubmitting(false);
      }}
      validationSchema={SettingsSchema}
    >
      <Form
        id={"settings"}
      >
        <Tabs variant={'line'}>
          <TabList>
            <Tab>General</Tab>
            <Tab>Proxy</Tab>
          </TabList>
          <TabPanels>
            <TabPanel>
              <GeneralSettingsTab />
            </TabPanel>
            <TabPanel>
              <ProxySettingsTab />
            </TabPanel>
          </TabPanels>
        </Tabs>
      </Form>
    </Formik>
  )
}

export default function SettingsDrawer(props: SettingsDrawerProps) {
  const { data, error } = useRPC<ISettingsList>("session.settings.list", {
    keys: [
      "listen_interfaces",
      "proxy_type",
      "proxy_hostname",
      "proxy_port",
      "proxy_username",
      "proxy_password",
      "proxy_hostnames",
      "proxy_peer_connections",
      "proxy_tracker_connections"
    ]
  });

  const [ submitting, setSubmitting ] = useState(false);

  return (
    <Drawer
      isOpen={props.isOpen}
      onClose={props.onClose}
      size={"lg"}
    >
      <DrawerOverlay />
      <DrawerContent>
        <DrawerCloseButton />
        <DrawerHeader>Settings</DrawerHeader>
        <DrawerBody>
          {
            error
              ? <div>{error.toString()}</div>
              : !data
                ? <Box><Spinner /></Box>
                : <SettingsForm onSubmitting={setSubmitting} settings={data.settings} />
          }
        </DrawerBody>
        <DrawerFooter>
          <Spinner visibility={submitting ? "visible" : "hidden"} mr={3} />
          <Button
            colorScheme={"purple"}
            disabled={submitting}
            type={"submit"}
            form={"settings"}
          >
            Save settings
          </Button>
        </DrawerFooter>
      </DrawerContent>
    </Drawer>
  )
}
