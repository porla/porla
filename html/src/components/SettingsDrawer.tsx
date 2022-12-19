import { Box, Button, Drawer, DrawerBody, DrawerCloseButton, DrawerContent, DrawerFooter, DrawerHeader, DrawerOverlay, propNames, Spinner, Tab, TabList, TabPanel, TabPanels, Tabs } from "@chakra-ui/react";
import { Form, Formik } from "formik";
import { useState } from "react";
import * as Yup from "yup";

import { ISettingsDict, ISettingsList } from "../types";
import { useInvoker, useRPC } from "../services/jsonrpc";
import GeneralSettingsTab from "./settings/GeneralSettingsTab";
import ProxySettingsTab from "./settings/ProxySettingsTab";
import QueueingSettingsTab from "./settings/QueueingSettingsTab";
import ConnectionSettingsTab from "./settings/NetworkSettingsTab";
import DiskIoSettingsTab from "./settings/DiskIoSettingsTab";

type SettingsDrawerProps = {
  isOpen: boolean;
  onClose: () => void;
}

const SettingsSchema = Yup.object().shape({
  active_checking: Yup.number().required("Required").min(-1, "Cannot be less than -1"),
  active_downloads: Yup.number().required("Required").min(-1, "Cannot be less than -1"),
  active_limit: Yup.number().required("Required").min(-1, "Cannot be less than -1"),
  active_seeds: Yup.number().required("Required").min(-1, "Cannot be less than -1"),
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
    }),
  torrent_connect_boost: Yup.number().required().max(255)
});

type SettingsFormProps = {
  onSubmitted: () => void;
  onSubmitting: (isSubmitting: boolean) => void;
  settings: ISettingsDict;
}

function SettingsForm(props: SettingsFormProps) {
  const { onSubmitted, onSubmitting, settings } = props;

  const sessionSettingsUpdate = useInvoker<void>("session.settings.update");

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

        await sessionSettingsUpdate({
          settings: delta
        });

        // Not even ashamed of this.
        await new Promise(r => setTimeout(r, 500));

        onSubmitting(false);
        onSubmitted();
      }}
      validationSchema={SettingsSchema}
    >
      <Form
        id={"settings"}
      >
        <Tabs variant={'line'}>
          <TabList>
            <Tab>General</Tab>
            <Tab>Disk I/O</Tab>
            <Tab>Network</Tab>
            <Tab>Proxy</Tab>
            <Tab>Queueing</Tab>
          </TabList>
          <TabPanels>
            <TabPanel>
              <GeneralSettingsTab />
            </TabPanel>
            <TabPanel>
              <DiskIoSettingsTab />
            </TabPanel>
            <TabPanel>
              <ConnectionSettingsTab />
            </TabPanel>
            <TabPanel>
              <ProxySettingsTab />
            </TabPanel>
            <TabPanel>
              <QueueingSettingsTab />
            </TabPanel>
          </TabPanels>
        </Tabs>
      </Form>
    </Formik>
  )
}

export default function SettingsDrawer(props: SettingsDrawerProps) {
  const { data, error, mutate } = useRPC<ISettingsList>("session.settings.list", {
    keys: [
      "active_checking",
      "active_downloads",
      "active_limit",
      "active_seeds",
      "allow_multiple_connections_per_ip",
      "allowed_fast_set_size",
      "announce_to_all_trackers",
      "auto_manage_interval",
      "auto_manage_prefer_seeds",
      "auto_scrape_interval",
      "auto_scrape_min_interval",
      "checking_mem_usage",
      "choking_algorithm",
      "connection_speed",
      "connections_limit",
      "disk_write_mode",
      "dont_count_slow_torrents",
      "file_pool_size",
      "hashing_threads",
      "inactive_down_rate",
      "inactive_up_rate",
      "inactivity_timeout",
      "incoming_starts_queued_torrents",
      "initial_picker_threshold",
      "listen_interfaces",
      "listen_queue_size",
      "max_allowed_in_request_queue",
      "max_failcount",
      "max_out_request_queue",
      "max_peer_recv_buffer_size",
      "max_queued_disk_bytes",
      "min_reconnect_time",
      "mixed_mode_algorithm",
      "max_rejects",
      "mmap_file_size_cutoff",
      "no_atime_storage",
      "peer_timeout",
      "peer_turnover",
      "peer_turnover_cutoff",
      "peer_turnover_interval",
      "predictive_piece_announce",
      "proxy_type",
      "proxy_hostname",
      "proxy_port",
      "proxy_username",
      "proxy_password",
      "proxy_hostnames",
      "proxy_peer_connections",
      "proxy_tracker_connections",
      "rate_choker_initial_threshold",
      "request_timeout",
      "seed_choking_algorithm",
      "seed_time_ratio_limit",
      "send_buffer_low_watermark",
      "send_buffer_watermark",
      "send_buffer_watermark_factor",
      "send_not_sent_low_watermark",
      "share_ratio_limit",
      "strict_end_game_mode",
      "suggest_mode",
      "torrent_connect_boost",
      "unchoke_slots_limit",
      "use_parole_mode",
      "whole_pieces_threshold"
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
                : <SettingsForm
                  onSubmitted={() => mutate()}
                  onSubmitting={setSubmitting}
                  settings={data.settings}
                />
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
