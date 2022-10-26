import { Box, Tab, TabList, TabPanel, TabPanels, Tabs } from '@chakra-ui/react';
import LibtorrentSettingsTab from '../components/settings/LibtorrentSettingsTab';
import ProxySettingsTab from '../components/settings/ProxySettingsTab';

export default function Settings() {
  return (
    <Box m={3}>
      <Tabs isLazy variant={'line'}>
        <TabList>
          <Tab>General</Tab>
          <Tab>Proxy</Tab>
          <Tab>Libtorrent settings</Tab>
        </TabList>
        <TabPanels>
          <TabPanel>
            Something
          </TabPanel>
          <TabPanel>
            <ProxySettingsTab />
          </TabPanel>
          <TabPanel>
            <LibtorrentSettingsTab />
          </TabPanel>
        </TabPanels>
      </Tabs>
    </Box>
  )
}
