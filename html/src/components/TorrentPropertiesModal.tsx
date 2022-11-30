import { Button } from "@chakra-ui/button";
import { FormControl, FormHelperText, FormLabel } from "@chakra-ui/form-control";
import { Box, Flex, VStack } from "@chakra-ui/layout";
import { Modal, ModalBody, ModalCloseButton, ModalContent, ModalFooter, ModalHeader, ModalOverlay } from "@chakra-ui/modal";
import { NumberInput, NumberInputField } from "@chakra-ui/number-input";
import { Spinner } from "@chakra-ui/spinner";
import { Switch } from "@chakra-ui/switch";
import { Field, Form, Formik } from "formik";
import { useEffect, useState } from "react";
import { useInvoker } from "../services/jsonrpc";
import { Torrent, TorrentProperties } from "../types";

type TorrentPropertiesModalProps = {
  torrent?: Torrent | null;
  onClose: () => void;
}

export default function TorrentPropertiesModal(props: TorrentPropertiesModalProps) {
  const [ torrentProps, setTorrentProps ] = useState<TorrentProperties>();
  const torrentsPropertiesGet = useInvoker<TorrentProperties>("torrents.properties.get");
  const torrentsPropertiesSet = useInvoker<{}>("torrents.properties.set");

  useEffect(() => {
    if (!props.torrent) {
      setTorrentProps(undefined);
      return;
    }

    torrentsPropertiesGet({
      info_hash: props.torrent.info_hash
    }).then(r => {
      setTorrentProps(r);
    });

  }, [props.torrent]);

  if (!props.torrent || !torrentProps) {
    return <></>
  }

  return (
    <Modal
      isOpen={!!props.torrent}
      onClose={props.onClose}
    >
      <ModalOverlay />
      <ModalContent>
        <ModalHeader>Properties</ModalHeader>
        <ModalCloseButton />
        <ModalBody>
          <Formik
            initialValues={{
              auto_managed: ((torrentProps.flags & (1<<5)) === 1<<5),
              download_limit: torrentProps.download_limit,
              max_connections: torrentProps.max_connections,
              max_uploads: torrentProps.max_uploads,
              sequential_download: ((torrentProps.flags & (1<<9)) === 1<<9),
              upload_limit: torrentProps.upload_limit
            }}
            onSubmit={async (values) => {
              let set_flags = 0;
              if (values.auto_managed) set_flags |= 1<<5;
              if (values.sequential_download) set_flags |= 1<<9;

              let unset_flags = 0;
              if (!values.auto_managed) unset_flags |= 1<<5;
              if (!values.sequential_download) unset_flags |= 1<<9;

              await torrentsPropertiesSet({
                info_hash: props.torrent?.info_hash,
                set_flags,
                unset_flags,
                ...values
              });

              props.onClose();
            }}
          >
            {({ values, setFieldValue }) => (
              <Form id="props">
                <VStack spacing={3}>
                  <Flex justifyContent={"space-between"} w={"100%"}>
                      <Box>Automatically managed</Box>
                      <Switch
                        isChecked={values.auto_managed}
                        onChange={e => setFieldValue("auto_managed", e.target.checked)}
                      />
                  </Flex>
                  <Flex justifyContent={"space-between"} w={"100%"}>
                      <Box>Sequential download</Box>
                      <Switch
                        isChecked={values.sequential_download}
                        onChange={e => setFieldValue("sequential_download", e.target.checked)}
                      />
                  </Flex>
                </VStack>
                <Field name="download_limit">
                  {(w: any) => (
                    <FormControl mt={3}>
                      <FormLabel>Download limit</FormLabel>
                      <NumberInput
                        onChange={(_, n) => setFieldValue("download_limit", n || 0)}
                        value={values.download_limit}
                      >
                        <NumberInputField value={values.download_limit} />
                      </NumberInput>
                      <FormHelperText>The download limit for this torrent. -1 means unlimited.</FormHelperText>
                    </FormControl>
                  )}
                </Field>
                <FormControl mt={3}>
                  <FormLabel>Max connections</FormLabel>
                  <NumberInput
                    onChange={(_, n) => setFieldValue("max_connections", n || 0)}
                    value={values.max_connections}
                  >
                    <NumberInputField />
                  </NumberInput>
                </FormControl>
                <FormControl mt={3}>
                  <FormLabel>Max uploads</FormLabel>
                  <NumberInput
                    onChange={(_, n) => setFieldValue("max_uploads", n || 0)}
                    value={values.max_uploads}
                  >
                    <NumberInputField />
                  </NumberInput>
                </FormControl>
                <FormControl mt={3}>
                  <FormLabel>Upload limit</FormLabel>
                  <NumberInput
                    onChange={(_, n) => setFieldValue("upload_limit", n || 0)}
                    value={values.upload_limit}
                  >
                    <NumberInputField />
                  </NumberInput>
                </FormControl>
              </Form>
            )}
          </Formik>
        </ModalBody>
        <ModalFooter>
          <Button
            colorScheme={"purple"}
            form={"props"}
            type={"submit"}
          >
            Save
          </Button>
        </ModalFooter>
      </ModalContent>
    </Modal>
  )
}