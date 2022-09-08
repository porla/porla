import React, { useRef, useState } from "react";
import { AlertDialog, AlertDialogBody, AlertDialogContent, AlertDialogFooter, AlertDialogHeader, AlertDialogOverlay, Button, Checkbox, useDisclosure } from "@chakra-ui/react";
import { useTranslation, Trans } from "react-i18next";
import { trpc } from "../utils/trpc";

interface IRemoveTorrentDialogProps {
  isOpen: boolean;
  onClose: () => void;
  onOpen: () => void;
  torrent: any;
}

export function RemoveTorrentDialog(props: IRemoveTorrentDialogProps) {
  const { t } = useTranslation();

  const cancelRef = useRef(null);
  const [removeFiles, setRemoveFiles] = useState<boolean>(false);
  const remove = trpc.useMutation("torrents.remove");
  const removeTranslation = !removeFiles ? 'remove_files_dialog' : 'remove_files_dialog_all';

  return (
    <AlertDialog
      isOpen={props.isOpen}
      leastDestructiveRef={cancelRef}
      onClose={props.onClose}
    >
      <AlertDialogOverlay>
        <AlertDialogContent>
          <AlertDialogHeader>
            {t("remove_torrent")}
          </AlertDialogHeader>
          <AlertDialogBody>
            {t(removeTranslation)}
          </AlertDialogBody>
          <AlertDialogFooter>
            <Checkbox flex={1} onChange={(e) => setRemoveFiles(e.target.checked)}>
              {t("remove_files")}
            </Checkbox>
            <Button ref={cancelRef} onClick={props.onClose}>
              {t("cancel")}
            </Button>
            <Button
              colorScheme={removeFiles ? "red" : "yellow"}
              ml={3}
              onClick={async () => {
                await remove.mutateAsync({
                  info_hash: props.torrent.info_hash,
                  remove_files: removeFiles
                });
                props.onClose();
              }}
            >
              {t("remove")}
            </Button>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialogOverlay>
    </AlertDialog>
  );
}
