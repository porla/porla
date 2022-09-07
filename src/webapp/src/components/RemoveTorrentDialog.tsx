import React, { useRef, useState } from "react";
import { AlertDialog, AlertDialogBody, AlertDialogContent, AlertDialogFooter, AlertDialogHeader, AlertDialogOverlay, Button, Checkbox, useDisclosure } from "@chakra-ui/react";
import { trpc } from "../utils/trpc";

interface IRemoveTorrentDialogProps {
  isOpen: boolean;
  onClose: () => void;
  onOpen: () => void;
  torrent: any;
}

export function RemoveTorrentDialog(props: IRemoveTorrentDialogProps) {
  const cancelRef = useRef(null);
  const [removeFiles,setRemoveFiles] = useState<boolean>(false);
  const remove = trpc.useMutation("torrents.remove");

  return (
    <AlertDialog
      isOpen={props.isOpen}
      leastDestructiveRef={cancelRef}
      onClose={props.onClose}
    >
      <AlertDialogOverlay>
        <AlertDialogContent>
          <AlertDialogHeader>Remove torrent</AlertDialogHeader>
          <AlertDialogBody>
            Are you sure? The torrent {removeFiles && <>and any downloaded data</>} will be removed from Porla.
          </AlertDialogBody>
          <AlertDialogFooter>
            <Checkbox flex={1} onChange={(e) => setRemoveFiles(e.target.checked)}>Remove files</Checkbox>
            <Button ref={cancelRef} onClick={props.onClose}>Cancel</Button>
            <Button
              colorScheme={removeFiles?"red":"yellow"}
              ml={3}
              onClick={async () => {
                await remove.mutateAsync({
                  info_hash: props.torrent.info_hash,
                  remove_files: removeFiles
                });

                props.onClose();
              }}
            >
              Remove
            </Button>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialogOverlay>
    </AlertDialog>
  );
}
