import { Alert, AlertDescription, AlertIcon, AlertTitle, Box, FormControl, FormHelperText, FormLabel, Input } from "@chakra-ui/react";
import jsonrpc from "../../services/jsonrpc";

type ISettingsDict = {
  proxy_hostname: string;
  proxy_hostnames: boolean;
  proxy_password: string;
  proxy_peer_connections: boolean;
  proxy_port: number;
  proxy_tracker_connections: boolean;
  proxy_type: number;
  proxy_username: string;
}

type ISettings = {
  settings: ISettingsDict;
}

export default function ProxySettingsTab() {
  const { data, error } = jsonrpc<ISettings>("session.settings.list", {
    keys: [
      "proxy_hostname",
      "proxy_port",
      "proxy_username",
      "proxy_password",
      "proxy_type",
      "proxy_hostnames",
      "proxy_peer_connections",
      "proxy_tracker_connections"
    ]
  });

  if (error) {
    return (
      <Alert status={"error"}>
        <AlertIcon />
        <AlertTitle>Failed to fetch proxy settings!</AlertTitle>
        <AlertDescription>An error occured when fetching proxy settings.</AlertDescription>
      </Alert>
    )
  }

  if (!data) {
    return <div>Loading...</div>
  }

  return (
    <Box>
      <FormControl>
        <FormLabel>Proxy host</FormLabel>
        <Input type={"text"} />
        <FormHelperText>The hostname or IP address.</FormHelperText>
      </FormControl>
      <FormControl mt={3}>
        <FormLabel>Proxy port</FormLabel>
        <Input type={"number"} />
        <FormHelperText>The port number to use.</FormHelperText>
      </FormControl>
    </Box>
  )
}
