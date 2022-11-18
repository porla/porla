import { Button, Checkbox, Flex, List, ListIcon, ListItem, Modal, ModalBody, ModalCloseButton, ModalContent, ModalFooter, ModalHeader, ModalOverlay } from "@chakra-ui/react";
import { useState } from "react";
import { MdWarning } from "react-icons/md";

type IRemoveTorrentModalProps = {
  torrent: any;
  onRemove: (torrent: any, removeData: boolean) => void;
}

export default function RemoveTorrentModal(props: IRemoveTorrentModalProps) {
  const [ removeFiles, setRemoveFiles ] = useState(false);

  return (
    <Modal isOpen={!!props.torrent} onClose={() => props.onRemove(null, false)}>
      <ModalOverlay />
      <ModalContent>
        <ModalHeader>Remove torrent</ModalHeader>
        <ModalCloseButton />
        <ModalBody>
          <p>This will remove the torrent from Porla. { removeFiles ? 'It will also remove any downloaded data.' : '' }</p>
          <List mt={2}>
            <ListItem>
              <ListIcon as={MdWarning} color={"yellow.300"} />
              {props.torrent?.name}
            </ListItem>
          </List>
        </ModalBody>
        <ModalFooter
          justifyContent={"space-between"}
        >
          <Checkbox isChecked={removeFiles} onChange={e => setRemoveFiles(e.target.checked)}>Remove data</Checkbox>
          <Button colorScheme={"red"} onClick={() => props.onRemove(props.torrent, removeFiles)}>Remove</Button>
        </ModalFooter>
      </ModalContent>
    </Modal>
  )
}
