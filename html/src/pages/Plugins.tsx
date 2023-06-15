import { Box, Button, Checkbox, FormControl, FormErrorMessage, FormHelperText, FormLabel, HStack, Heading, Input, Modal, ModalBody, ModalContent, ModalFooter, ModalHeader, ModalOverlay, Select, Textarea } from "@chakra-ui/react";
import { Field, Form, Formik } from "formik";
import { useState } from "react";
import { useInvoker } from "../services/jsonrpc";

export default function Plugins() {
  const installPlugin = useInvoker<any>("plugins.install");

  const [ showInstall, setShowInstall ] = useState(false);

  return (
    <Box>
      <Modal isOpen={showInstall} onClose={() => setShowInstall(false)} size={"lg"}>
        <ModalOverlay />
        <ModalContent>
          <Formik
            initialValues={{
              source: 'dir',
              path: '',
              enable: true
            }}
            onSubmit={async (values, { setSubmitting }) => {
              setSubmitting(true);

              await installPlugin(values);

              setShowInstall(false);
              setSubmitting(false);
            }}
          >
            {({ errors, isSubmitting, setFieldValue, touched, values }) => (
              <Form>
                <ModalHeader>Install plugin</ModalHeader>
                <ModalBody>
                  <Field name="source">
                    {(w: any) => (
                      <FormControl>
                        <FormLabel>Source</FormLabel>
                        <Select
                          {...w.field}
                        >
                          <option value={'dir'}>Folder</option>
                          <option value={'git'}>Git</option>
                        </Select>
                      </FormControl>
                    )}
                  </Field>
                  <Field name="path">
                    {(w: any) => (
                      <FormControl mt={5}>
                        <FormLabel>
                          {values.source === 'dir' ? 'Path' : 'Repository'}
                        </FormLabel>
                        <Input {...w.field} type="text" />
                        {
                          errors.path && touched.path
                            ? <FormErrorMessage>{errors.path}</FormErrorMessage>
                            : values.source === 'dir'
                              ? <FormHelperText>The path where the plugin is located.</FormHelperText>
                              : <FormHelperText>A HTTPS URL to a Git repository.</FormHelperText>
                        }
                      </FormControl>
                    )}
                  </Field>
                  <Field name="config">
                    {(w: any) => (
                      <FormControl mt={5}>
                        <FormLabel>Configuration</FormLabel>
                        <Textarea
                          {...w.field}
                          fontFamily={"monospace"}
                          fontSize={"sm"}
                          rows={10}
                        >
                        </Textarea>
                        <FormHelperText>Any configuration that the plugin expects. Refer to the documentation of the plugin.</FormHelperText>
                      </FormControl>
                    )}
                  </Field>
                </ModalBody>
                <ModalFooter justifyContent={"space-between"}>
                  <Field name="enable">
                    {(w: any) => (
                      <Checkbox
                        {...w.field}
                        colorScheme="purple"
                        isChecked={values.enable}
                        onChange={e => setFieldValue("enable", e.target.checked)}
                      >
                        Enable
                      </Checkbox>
                    )}
                  </Field>
                  <Button
                    colorScheme="purple"
                    disabled={isSubmitting}
                    type="submit"
                  >
                    Install
                  </Button>
                </ModalFooter>
              </Form>
            )}
          </Formik>
        </ModalContent>
      </Modal>

      <HStack m={3}>
        <Heading as="h3" size={"md"}>Plugins</Heading>
        <Button
          colorScheme={"purple"}
          size={"xs"}
          variant={"outline"}
          onClick={() => setShowInstall(true)}
        >
          Install new
        </Button>
      </HStack>
    </Box>
  )
}
