import { Box, Button, Checkbox, FormControl, FormErrorMessage, FormHelperText, FormLabel, HStack, Heading, IconButton, Input, Menu, MenuButton, MenuDivider, MenuItem, MenuList, Modal, ModalBody, ModalContent, ModalFooter, ModalHeader, ModalOverlay, Select, Table, Tbody, Td, Textarea, Th, Thead, Tr } from "@chakra-ui/react";
import { Field, Form, Formik } from "formik";
import { useState } from "react";
import { useInvoker, useRPC } from "../services/jsonrpc";
import { MdExtensionOff, MdOutlineMoreVert, MdSettings } from "react-icons/md";

type IVersionInfo = {
  head_name: string;
  shorthand: string;
}

type IPlugin = {
  name: string;
  path: string;
  version_info: IVersionInfo | null;
}

type IPluginsList = {
  plugins: IPlugin[];
}

export default function Plugins() {
  const installPlugin = useInvoker<any>("plugins.install");
  const uninstallPlugin = useInvoker<any>("plugins.uninstall");

  const { data, error } = useRPC<IPluginsList>("plugins.list");

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

      <Table>
        <Thead>
          <Tr>
            <Th>Name</Th>
            <Th>Path</Th>
            <Th>Version</Th>
            <Th></Th>
          </Tr>
        </Thead>
        <Tbody>
          {data?.plugins.map(p => (
            <Tr>
              <Td>{p.name}</Td>
              <Td>{p.path}</Td>
              <Td>{p.version_info?.head_name} ({p.version_info?.shorthand})</Td>
              <Td>
                <Menu>
                  <MenuButton
                    as={IconButton}
                    icon={<MdOutlineMoreVert />}
                    size={"sm"}
                  />
                  <MenuList>
                    <MenuItem
                      icon={<MdSettings />}
                    >
                      Configure
                    </MenuItem>
                    <MenuDivider />
                    <MenuItem
                      icon={<MdExtensionOff />}
                      onClick={async () => {
                        await uninstallPlugin({
                          name: p.name
                        })
                      }}
                    >
                      Uninstall
                    </MenuItem>
                  </MenuList>
                </Menu>
              </Td>
            </Tr>
          ))}
        </Tbody>
      </Table>
    </Box>
  )
}
