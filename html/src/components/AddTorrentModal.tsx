import { Alert, AlertDescription, AlertIcon, AlertTitle, Box, Button, FormControl, FormErrorMessage, FormHelperText, FormLabel, Input, Modal, ModalBody, ModalCloseButton, ModalContent, ModalFooter, ModalHeader, ModalOverlay, Select, Tab, TabList, TabPanel, TabPanels, Tabs } from "@chakra-ui/react";
import { Field, Form, Formik } from "formik";
import * as Yup from "yup";

import { InfoHash, PresetsList } from "../types/index";
import { RpcError, useInvoker } from "../services/jsonrpc";
import { useEffect, useState } from "react";
import { filesize } from "filesize";

const readSingleFile = (blob: Blob): Promise<string> => {
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
  });
}

const readFiles = async (fileList: FileList): Promise<string[]> => {
  let files = [];
  for (const blob of fileList) {
    files.push(await readSingleFile(blob));
  }
  return files;
}

const AddTorrentSchema = Yup.object().shape({
  magnet_uri: Yup.string()
    .when("type", {
      is: (t: string) => t === "magnet_uri",
      then: Yup.string().required("Magnet link is required")
    }),
  save_path: Yup.string()
    .required("Save path is required."),
  ti: Yup.mixed()
    .when("type", {
      is: (t: string) => t === "torrent",
      then: Yup.mixed().required("A torrent file is required")
    }),
  type: Yup.string()
    .oneOf(["torrent","magnet_uri"])
    .required()
});

type AddTorrentModalProps = {
  isOpen: boolean;
  onClose: (hashes?: InfoHash[]) => void;
  presets: PresetsList;
}

export default function AddTorrentModal(props: AddTorrentModalProps) {
  const { presets } = props.presets;

  const fsSpace             = useInvoker<any>("fs.space");
  const torrentsAdd         = useInvoker<InfoHash>("torrents.add");
  const [ error, setError ] = useState<any>();
  const [ path, setPath ]   = useState<string | null>(null);
  const [ space, setSpace ] = useState<any>();

  useEffect(() => {
    if (path) {
      fsSpace({ path })
        .then(r => setSpace(() => r))
        .catch(() => setSpace(() => null));
    }
  }, [path]);

  useEffect(() => {
    if (presets.default && presets.default.save_path) {
      setPath(() => presets.default.save_path!);
    }
  }, []);

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
            preset: "default" in presets ? "default" : "",
            save_path: "default" in presets
              ? presets.default.save_path
              : "",
            ti: []
          }}
          onSubmit={async (values) => {
            const params: any = {
              preset:    values.preset === "" ? "" : values.preset,
              save_path: values.save_path,
            };

            if (values.type === "magnet_uri") {
              params.magnet_uri = values.magnet_uri;
              return props.onClose([await torrentsAdd(params)]);
            }

            let addedHashes: InfoHash[] = [];

            if (values.ti.length === 1) {
              params.ti = values.ti[0];

              try {
                addedHashes.push(await torrentsAdd(params));
                props.onClose(addedHashes);
              } catch (err) {
                setError(err);
                setTimeout(() => setError(undefined), 3000);
              }

              return;
            }

            for (const ti of values.ti) {
              params.ti = ti;

              try {
                addedHashes.push(await torrentsAdd(params));
              } catch(err) {
                console.error(err);
              }
            }

            props.onClose(addedHashes);
          }}
          validationSchema={AddTorrentSchema}
        >
          {({ errors, setFieldValue, isSubmitting, touched, values }) => (
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
                              multiple
                              type="file"
                              onChange={async e => {
                                if (e.currentTarget.files != null) {
                                  setFieldValue("ti", await readFiles(e.currentTarget.files));
                                }
                              }}
                            />
                            {
                              errors.ti && touched.ti
                                ? <FormErrorMessage>{errors.ti}</FormErrorMessage>
                                : <FormHelperText>One (or more) torrent files. All files will be added with the same settings.</FormHelperText>
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
                              setPath(() => presets[e.target.value].save_path);
                            }
                          }}
                        >
                          { Object.keys(presets).map(p => (
                            <option key={p} selected={p === values.preset}>{p}</option>
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
                        onChange={e => {
                          setFieldValue("save_path", e.target.value);
                          setPath(() => e.target.value);
                        }}
                      />
                      {
                        errors.save_path && touched.save_path
                          ? <FormErrorMessage>{errors.save_path}</FormErrorMessage>
                          : <FormHelperText>
                              {
                                space && space.quota
                                  ? <>
                                      Available quota:
                                        <strong>
                                          {filesize((space.quota.blocks_limit_hard * 8) - space.quota.current_space).toString()}
                                        </strong>
                                    </>
                                  : space
                                    ? <>
                                        Available space: <strong>{filesize(space.available).toString()}</strong> 
                                      </>
                                    : <>The path on disk where the torrent will be saved.</>
                              }
                            </FormHelperText>
                      }
                    </FormControl>
                  )}
                </Field>
              </ModalBody>
              <ModalFooter justifyContent={"space-between"}>
                <Box flex={1} mr={4}>
                  <Alert status='error' visibility={error ? "visible" : "hidden"}>
                    <AlertIcon />
                    <AlertDescription fontSize={"sm"}>{error?.message}</AlertDescription>
                  </Alert>
                </Box>
                <Button
                  colorScheme={"purple"}
                  disabled={isSubmitting}
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
