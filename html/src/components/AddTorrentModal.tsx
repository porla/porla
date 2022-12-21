import { Button, FormControl, FormErrorMessage, FormHelperText, FormLabel, Input, Modal, ModalBody, ModalCloseButton, ModalContent, ModalFooter, ModalHeader, ModalOverlay, Select, Tab, TabList, TabPanel, TabPanels, Tabs } from "@chakra-ui/react";
import { Field, Form, Formik } from "formik";
import useSWR from "swr";
import * as Yup from "yup";

import { InfoHash, PresetsList } from "../types/index";
import { useInvoker } from "../services/jsonrpc";

const readFile = (blob: Blob): Promise<string> => {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = function () {
      const idx = reader.result?.toString().indexOf("base64,")!;
      const data = reader.result?.toString().substring(idx + "base64,".length);
      if (!data) return reject();
      resolve(data);
    };
    reader.onerror = () => reject();
    reader.readAsDataURL(blob);
  })
}

const AddTorrentSchema = Yup.object().shape({
  magnet_uri: Yup.string()
    .when("type", {
      is: (t: any) => t === "magnet_uri",
      then: Yup.string().required("Magnet link is required")
    }),
  save_path: Yup.string()
    .required("Save path is required."),
  ti: Yup.mixed()
    .when("type", {
      is: (t: any) => t === "torrent",
      then: Yup.mixed().required("A torrent file is required")
    }),
  type: Yup.string()
    .oneOf(["torrent","magnet_uri"])
    .required()
});

type AddTorrentModalProps = {
  isOpen: boolean;
  onClose: (hash?: InfoHash) => void;
  presets: PresetsList;
}

export default function AddTorrentModal(props: AddTorrentModalProps) {
  const { presets } = props;

  const torrentsAdd = useInvoker<InfoHash>("torrents.add");

  return (
    <Modal
      isOpen={props.isOpen}
      onClose={() => props.onClose()}
      size={"xl"}
    >
      <ModalOverlay />
      <ModalContent>
        <Formik
          initialValues={{
            type: "torrent",
            magnet_uri: "",
            preset: "",
            save_path: "default" in presets
              ? presets.default.save_path
              : "",
            ti: ""
          }}
          onSubmit={async (values) => {
            const params: any = {
              save_path: values.save_path,
            };

            if (values.preset !== "") {
              params.preset = values.preset;
            }

            switch (values.type) {
              case "magnet_uri":
                params.magnet_uri = values.magnet_uri;
                break;
              case "torrent":
                params.ti = values.ti;
                break;
            }

            const hash = await torrentsAdd(params);

            props.onClose(hash);
          }}
          validationSchema={AddTorrentSchema}
        >
          {({ errors, initialValues, isValid, setFieldValue, touched, values }) => (
            <Form>
              <ModalHeader>Add {values.type === "torrent" ? "torrent" : "magnet link"}</ModalHeader>
              <ModalCloseButton />
              <ModalBody>
                <Tabs
                  onChange={index => {
                    setFieldValue("type", index === 0 ? "torrent" : "magnet_uri");
                  }}
                >
                  <TabList>
                    <Tab>Torrent file</Tab>
                    <Tab>Magnet link</Tab>
                  </TabList>
                  <TabPanels>
                    <TabPanel px={0}>
                      <Field name="ti">
                        {() => (
                          <FormControl isInvalid={touched.ti && !!errors.ti}>
                            <FormLabel>File</FormLabel>
                            <Input
                              type="file"
                              onChange={async e => {
                                if (e.currentTarget.files != null) {
                                  setFieldValue("ti", await readFile(e.currentTarget.files[0]));
                                }
                              }}
                            />
                            {
                              errors.ti && touched.ti
                                ? <FormErrorMessage>{errors.ti}</FormErrorMessage>
                                : <FormHelperText>A torrent file.</FormHelperText>
                            }
                          </FormControl>
                        )}
                      </Field>
                    </TabPanel>
                    <TabPanel px={0}>
                      <Field name="magnet_uri">
                        {(w: any) => (
                          <FormControl isInvalid={touched.magnet_uri && !!errors.magnet_uri}>
                            <FormLabel>Magnet link</FormLabel>
                            <Input
                              type="text"
                              {...w.field}
                              placeholder="magnet:?xt:btih:..."
                            />
                            {
                              errors.magnet_uri && touched.magnet_uri
                                ? <FormErrorMessage>{errors.magnet_uri}</FormErrorMessage>
                                : <FormHelperText>Any magnet link or info hash.</FormHelperText>
                            }
                          </FormControl>
                        )}
                      </Field>
                    </TabPanel>
                  </TabPanels>
                </Tabs>
                { presets && Object.keys(presets).length > 0 && !(Object.keys(presets).length === 1 && presets.default) && (
                  <Field name="preset">
                    {() => (
                      <FormControl mb={3}>
                        <FormLabel>Preset</FormLabel>
                        <Select
                          onChange={e => {
                            setFieldValue("preset", e.target.value);

                            // When changing preset, only update the values that are still
                            // their original value.
                            if (!touched.save_path) {
                              setFieldValue("save_path", presets[e.target.value].save_path);
                            }
                          }}
                        >
                          { Object.keys(presets).map(p => (
                            <option key={p}>{p}</option>
                          ))}
                        </Select>
                        <FormHelperText>Select a preset to apply to the torrent.</FormHelperText>
                      </FormControl>
                    )}
                  </Field>
                )}
                <Field name="save_path">
                  {(w: any) => (
                    <FormControl isInvalid={touched.save_path && !!errors.save_path}>
                      <FormLabel>Save path</FormLabel>
                      <Input
                        type="text"
                        {...w.field}
                      />
                      {
                        errors.save_path && touched.save_path
                          ? <FormErrorMessage>{errors.save_path}</FormErrorMessage>
                          : <FormHelperText>The path on disk where the torrent will be saved.</FormHelperText>
                      }
                    </FormControl>
                  )}
                </Field>
              </ModalBody>
              <ModalFooter>
                <Button
                  colorScheme={"purple"}
                  disabled={Object.keys(touched).length === 0 || !isValid}
                  type="submit"
                >
                  Add {values.type === "torrent" ? "torrent" : "magnet link"}
                </Button>
              </ModalFooter>
            </Form>
          )}
        </Formik>
      </ModalContent>
    </Modal>
  )
}
