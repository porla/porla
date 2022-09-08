import React from "react";
import { Button, FormControl, FormHelperText, FormLabel, Input, Modal, ModalBody, ModalCloseButton, ModalContent, ModalFooter, ModalHeader, ModalOverlay } from "@chakra-ui/react";
import { useTranslation } from "react-i18next";
import { MdDriveFileMove } from "react-icons/md";
import { Formik, Field } from "formik";
import { trpc } from "../utils/trpc";

interface IMoveTorrentModalProps {
  isOpen: boolean;
  onClose: () => void;
  onOpen: () => void;
  torrent: any;
}

export function MoveTorrentModal(props: IMoveTorrentModalProps) {
  const { t } = useTranslation();

  const move = trpc.useMutation("torrents.move");

  return (
    <Modal isOpen={props.isOpen} onClose={props.onClose} size={"lg"}>
      <ModalOverlay />
      <Formik
        initialValues={{
          path: props.torrent?.save_path || ""
        }}
        onSubmit={async (values) => {
          await move.mutateAsync({
            info_hash: props.torrent.info_hash,
            new_path: values.path
          });

          props.onClose();
        }}
      >
        {({ handleSubmit }) => (
          <form onSubmit={handleSubmit}>
            <ModalContent>
              <ModalHeader>
                {t("move_torrent")}
              </ModalHeader>
              <ModalCloseButton />
              <ModalBody>
                <FormControl>
                  <FormLabel htmlFor="path">
                    {t("target_path")}
                  </FormLabel>
                  <Field
                    as={Input}
                    id="path"
                    name="path"
                    placeholder="/mnt/new/path"
                    required
                    type="text"
                  />
                  <FormHelperText>
                    {t("target_path_helper")}
                  </FormHelperText>
                </FormControl>
              </ModalBody>
              <ModalFooter>
                <Button
                  colorScheme={"blue"}
                  leftIcon={<MdDriveFileMove />}
                  type="submit"
                >
                  {t("move")}
                </Button>
              </ModalFooter>
            </ModalContent>
          </form>
        )}
      </Formik>
    </Modal>
  );
}
