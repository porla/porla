import { Button, Modal, ModalBody, ModalCloseButton, ModalContent, ModalFooter, ModalHeader, ModalOverlay, useDisclosure } from "@chakra-ui/react";

export default function RemoveTorrentModal() {
  const { isOpen, onOpen, onClose } = useDisclosure();

  return (
    <Modal isOpen={isOpen} onClose={onClose}>
      <ModalOverlay />
      <ModalContent>
        <ModalHeader>Remove torrent</ModalHeader>
        <ModalCloseButton />
        <ModalBody>
          <p>Hello</p>
        </ModalBody>
        <ModalFooter>
          <Button colorScheme={"blue"} onClick={onClose}>Remove</Button>
        </ModalFooter>
      </ModalContent>
    </Modal>
  )
}