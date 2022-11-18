import { Button, FormControl, FormErrorMessage, FormHelperText, FormLabel, Input, Modal, ModalBody, ModalCloseButton, ModalContent, ModalFooter, ModalHeader, ModalOverlay, Select, Tab, TabList, TabPanel, TabPanels, Tabs } from "@chakra-ui/react";
import { Field, Form, Formik } from "formik";
import useSWR from "swr";
import * as Yup from "yup";

import porla, { InfoHash } from "../services/index";

const readFile = (blob: Blob): Promise<string> => {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = function () {
      const idx = reader.result?.toString().indexOf("base64,")!;
      const data = reader.result?.toString().substring(idx + "base64,".length);
      if (!data) return reject();
      resolve(data);
    };
    reader.onerror = (ev) => reject();
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
}

export default function AddTorrentModal(props: AddTorrentModalProps) {
  const {
    data: presets,
    error: presets_error
  } = useSWR("presets", porla.presets.list);

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
            save_path: "",
            ti: ""
          }}
          onSubmit={async (values, actions) => {
            actions.setSubmitting(true);

            const params: any = {
              save_path: values.save_path,
            };

            switch (values.type) {
              case "magnet_uri":
                params.magnet_uri = values.magnet_uri;
                break;
              case "torrent":
                params.ti = values.ti;
                break;
            }

            const hash = await porla.torrents.add(params);
            actions.setSubmitting(false);
            props.onClose(hash);
          }}
          validationSchema={AddTorrentSchema}
        >
          {({ errors, isValid, setFieldValue, touched, values }) => (
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
                        {(w: any) => (
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
                { presets && Object.keys(presets).length > 0 && (
                  <Field name="preset">
                    {(w: any) => (
                      <FormControl>
                        <FormLabel>Preset</FormLabel>
                        <Select
                        >
                          { Object.keys(presets).map(p => (
                            <option key={p}>{p}</option>
                          ))}
                        </Select>
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
