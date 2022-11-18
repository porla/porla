import { Box, Checkbox, Flex, FormControl, FormErrorMessage, FormHelperText, FormLabel, Input, Select, Spacer } from "@chakra-ui/react";
import { Field, useFormikContext } from "formik";

type IFormData = {
  proxy_type: number;
  proxy_hostname: string;
  proxy_port: number;
  proxy_username: string;
  proxy_password: string;
  proxy_hostnames: boolean;
  proxy_peer_connections: boolean;
  proxy_tracker_connections: boolean;
}

export default function ProxySettingsTab() {
  const {
    errors,
    handleChange,
    setFieldValue,
    touched,
    values
  } = useFormikContext<IFormData>();

  return (
    <Box>
      <Field name="proxy_type">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Type</FormLabel>
            <Select
              {...w.field}
              onChange={e => {
                setFieldValue("proxy_type", parseInt(e.target.value))
              }}
            >
              <option value={0}>No proxy</option>
              <option value={1}>SOCKS4</option>
              <option value={2}>SOCKS5</option>
              <option value={3}>SOCKS5 w/ auth</option>
              <option value={4}>HTTP</option>
              <option value={5}>HTTP w/ auth</option>
              <option value={6}>i2p SAM</option>
            </Select>
            <FormHelperText>The type of proxy to use.</FormHelperText>
          </FormControl>
        )}
      </Field>
      { values.proxy_type > 0 && (
        <>
          <Field name="proxy_hostname">
            {(w: any) => (
              <FormControl isInvalid={!!errors.proxy_hostname && touched.proxy_hostname} mb={3}>
                <FormLabel>Host</FormLabel>
                <Input
                  type={"text"}
                  {...w.field}
                />
                {
                  errors.proxy_hostname && touched.proxy_hostname
                    ? <FormErrorMessage>{errors.proxy_hostname}</FormErrorMessage>
                    : <FormHelperText>The hostname or IP address.</FormHelperText>
                }
              </FormControl>
            )}
          </Field>
          <Field name="proxy_port">
            {(w: any) => (
              <FormControl isInvalid={!!errors.proxy_port && touched.proxy_port} mb={5}>
                <FormLabel>Port</FormLabel>
                <Input
                  type={"number"}
                  {...w.field}
                />
                {
                  errors.proxy_port && touched.proxy_port
                    ? <FormErrorMessage>{errors.proxy_port}</FormErrorMessage>
                    : <FormHelperText>The port number to use.</FormHelperText>
                }
              </FormControl>
            )}
          </Field>
          { (values.proxy_type == 3 || values.proxy_type == 5) && (
            <>
              <Field name="proxy_username">
                {(w: any) => (
                  <FormControl mb={5}>
                    <FormLabel>Username</FormLabel>
                    <Input
                      type={"text"}
                      {...w.field}
                    />
                    <FormHelperText>Username to authenticate against proxy.</FormHelperText>
                  </FormControl>
                )}
              </Field>
              <Field name="proxy_password">
                {(w: any) => (
                  <FormControl mb={5}>
                    <FormLabel>Password</FormLabel>
                    <Input
                      type={"password"}
                      {...w.field}
                    />
                    <FormHelperText>Password to authenticate against proxy.</FormHelperText>
                  </FormControl>
                )}
              </Field>
            </>
          )}
          <Flex>
            <Field name="proxy_hostnames">
              {(w: any) => (
                <Checkbox
                  id={"proxy_hostnames"}
                  name={"proxy_hostnames"}
                  onChange={handleChange}
                  isChecked={values.proxy_hostnames}
                >
                  Hostnames
                </Checkbox>
              )}
            </Field>
            <Spacer />
            <Field name="proxy_peer_connections">
              {(w: any) => (
                <Checkbox
                  id={"proxy_peer_connections"}
                  name={"proxy_peer_connections"}
                  onChange={handleChange}
                  isChecked={values.proxy_peer_connections}
                >
                  Peer connections
                </Checkbox>
              )}
            </Field>
            <Spacer />
            <Field name="proxy_tracker_connections">
              {(w: any) => (
                <Checkbox
                  id={"proxy_tracker_connections"}
                  name={"proxy_tracker_connections"}
                  onChange={handleChange}
                  isChecked={values.proxy_tracker_connections}
                >
                  Tracker connections
                </Checkbox>
              )}
            </Field>
          </Flex>
        </>
      )}
    </Box>
  )
}
