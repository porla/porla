import { Box, Button, FormControl, FormHelperText, FormLabel, Input, Modal, ModalBody, ModalCloseButton, ModalContent, ModalFooter, ModalHeader, ModalOverlay, Text } from "@chakra-ui/react";
import { Field, Form, Formik } from "formik";
import { Torrent } from "../types";

type IMoveTorrentModalProps = {
  torrent?: Torrent;
  onMove: (torrent: any, save_path?: string) => void;
}

export default function MoveTorrentModal(props: IMoveTorrentModalProps) {
  if (!props.torrent) {
    return <></>;
  }

  return (
    <Modal isOpen={!!props.torrent} onClose={() => props.onMove(null)}>
      <ModalOverlay />
      <ModalContent>
        <ModalHeader>Move torrent</ModalHeader>
        <ModalCloseButton />
        <ModalBody>
          <Formik
            initialValues={{
              save_path: props.torrent.save_path
            }}
            onSubmit={(values) => {
              props.onMove(props.torrent, values.save_path);
            }}
          >
            <Form id="move">
              <Field name="save_path">
                {(w: any) => (
                  <FormControl>
                    <FormLabel>Path</FormLabel>
                    <Input {...w.field} />
                    <FormHelperText>The new save path for this torrent.</FormHelperText>
                  </FormControl>
                )}
              </Field>
            </Form>
          </Formik>
        </ModalBody>
        <ModalFooter>
          <Button colorScheme={"purple"} form={"move"} type={"submit"}>Move</Button>
        </ModalFooter>
      </ModalContent>
    </Modal>
  )
}
