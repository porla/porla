import { Alert, AlertDescription, AlertIcon, AlertTitle, Button, Flex, HStack, Icon, Link, Modal, ModalBody, ModalContent, ModalFooter, ModalHeader, ModalOverlay } from "@chakra-ui/react";
import { MdErrorOutline, MdOpenInNew } from "react-icons/md";

type IAppErrorModalProps = {
  error: Error | undefined;
}

export default function AppErrorModal(props: IAppErrorModalProps) {
  return (
    <Modal isOpen={true} onClose={() => {}} size={"lg"}>
      <ModalOverlay />
      <ModalContent>
        <ModalHeader>
          <HStack spacing={2}>
            <Icon
              as={MdErrorOutline}
              color={"red.400"}
            />
            <span>Porla experienced an error</span>
          </HStack>
        </ModalHeader>
        <ModalBody>
          <p>
            An error has occured when connecting to the Porla API. Please review your
            web server configuration.
          </p>
          { props.error && (
            <Alert status="error" mt={3}>
              <AlertIcon />
              <AlertTitle>{props.error.toString()}</AlertTitle>
            </Alert>
          )}
        </ModalBody>
        <ModalFooter>
            <Button
              as={Link}
              rightIcon={<Icon as={MdOpenInNew} />}
              href="https://porla.org/"
              isExternal
              variant={"link"}
            >
              Go to documentation
            </Button>
        </ModalFooter>
      </ModalContent>
    </Modal>
  )
}
