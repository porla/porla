import { Box, Checkbox, Input, Tab, Table, TabList, TabPanel, TabPanels, Tabs, Tbody, Td, Th, Thead, Tr } from '@chakra-ui/react';
import jsonrpc from '../services/jsonrpc';

type ISettingsDict = {
  active_checking: number;
  active_dht_limit: number;
  active_downloads: number;
  active_limit: number;
  active_lsd_limit: number;
  active_seeds: number;
  active_tracker_limit: number;
  aio_threads: number;
  allow_i2p_mixed: boolean;
  allow_idna: boolean;
  allow_multiple_connections_per_ip: boolean;
  allowed_enc_level: number;
  allowed_fast_set_size: number;
  always_send_user_agent: boolean;
  announce_crypto_support: boolean;
  announce_ip: string;
  announce_to_all_tiers: boolean;
  announce_to_all_trackers: boolean;
  anonymous_mode: boolean;
  apply_ip_filter_to_trackers: boolean;
  auto_manage_interval: number;
  auto_manage_prefer_seeds: boolean;
  auto_manage_startup: number;
  auto_scrape_interval: number;
  auto_scrape_min_interval: number;
  auto_sequential: boolean;
  ban_web_seeds: boolean;
  checking_mem_usage: number;
  choking_algorithm: number;
  close_file_interval: number;
  close_redundant_connections: boolean;
  connect_seed_every_n_download: number;
  connection_speed: number;
  connections_limit: number;
  connections_slack: number;
  dht_aggressive_lookups: boolean;
  dht_announce_interval: number;
  dht_block_ratelimit: number;
  dht_block_timeout: number;
  dht_bootstrap_nodes: string;
  dht_enforce_node_id: boolean;
  dht_extended_routing_table: boolean;
  dht_ignore_dark_internet: boolean;
  dht_item_lifetime: number;
  dht_max_dht_items: number;
  dht_max_fail_count: number;
  dht_max_infohashes_sample_count: number;
  dht_max_peers: number;
  dht_max_peers_reply: number;
  dht_max_torrent_search_reply: number;
  dht_max_torrents: number;
  dht_prefer_verified_node_ids: boolean;
  dht_privacy_lookups: boolean;
  dht_read_only: boolean;
  dht_restrict_routing_ips: boolean;
  dht_restrict_search_ips: boolean;
  dht_sample_infohashes_interval: number;
  dht_search_branching: number;
  dht_upload_rate_limit: number;
  disable_hash_checks: boolean;
  disk_io_read_mode: number;
  disk_io_write_mode: number;
  dont_count_slow_torrents: boolean;
  download_rate_limit: number;
  enable_dht: boolean;
  enable_incoming_tcp: boolean;
  enable_incoming_utp: boolean;
  enable_ip_notifier: boolean;
  enable_lsd: boolean;
  enable_natpmp: boolean;
  enable_outgoing_tcp: boolean;
  enable_outgoing_utp: boolean;
  enable_set_file_valid_data: boolean;
  enable_upnp: boolean;
  file_pool_size: number;
  handshake_client_version: string;
  handshake_timeout: number;
  hashing_threads: number;
  i2p_hostname: string;
  i2p_port: number;
  in_enc_policy: number;
  inactive_down_rate: number;
  inactive_up_rate: number;
  inactivity_timeout: number;
  incoming_starts_queued_torrents: boolean;
  initial_picker_threshold: number;
  listen_interfaces: string;
  listen_queue_size: number;
  listen_system_port_fallback: boolean;
  local_service_announce_interval: number;
  max_allowed_in_request_queue: number;
  max_concurrent_http_announces: number;
  max_failcount: number;
  max_http_recv_buffer_size: number;
  max_metadata_size: number;
  max_out_request_queue: number;
  max_paused_peerlist_size: number;
  max_peer_recv_buffer_size: number;
  max_peerlist_size: number;
  max_pex_peers: number;
  max_piece_count: number;
  max_queued_disk_bytes: number;
  max_rejects: number;
  max_retry_port_bind: number;
  max_suggest_pieces: number;
  max_web_seed_connections: number;
  metadata_token_limit: number;
  min_announce_interval: number;
  min_reconnect_time: number;
  mixed_mode_algorithm: number;
  no_atime_storage: boolean;
  no_connect_privileged_ports: boolean;
  no_recheck_incomplete_resume: boolean;
  num_optimistic_unchoke_slots: number;
  num_outgoing_ports: number;
  num_want: number;
  optimistic_disk_retry: number;
  optimistic_unchoke_interval: number;
  out_enc_policy: number;
  outgoing_interfaces: string;
  outgoing_port: number;
  peer_connect_timeout: number;
  peer_dscp: number;
  peer_fingerprint: string;
  peer_timeout: number;
  peer_turnover: number;
  peer_turnover_cutoff: number;
  peer_turnover_interval: number;
  piece_extent_affinity: boolean;
  piece_timeout: number;
  predictive_piece_announce: number;
  prefer_rc4: boolean;
  prefer_udp_trackers: boolean;
  prioritize_partial_pieces: boolean;
  proxy_hostname: string;
  proxy_hostnames: boolean;
  proxy_password: string;
  proxy_peer_connections: boolean;
  proxy_port: number;
  proxy_tracker_connections: boolean;
  proxy_type: number;
  proxy_username: string;
  rate_choker_initial_threshold: number;
  rate_limit_ip_overhead: boolean;
  recv_socket_buffer_size: number;
  report_redundant_bytes: boolean;
  report_true_downloaded: boolean;
  report_web_seed_downloads: boolean;
  request_queue_time: number;
  request_timeout: number;
  resolver_cache_timeout: number;
  seed_choking_algorithm: number;
  seed_time_limit: number;
  seed_time_ratio_limit: number;
  seeding_outgoing_connections: boolean;
  seeding_piece_quota: number;
  send_buffer_low_watermark: number;
  send_buffer_watermark: number;
  send_buffer_watermark_factor: number;
  send_not_sent_low_watermark: number;
  send_redundant_have: boolean;
  send_socket_buffer_size: number;
  share_mode_target: number;
  share_ratio_limit: number;
  smooth_connects: boolean;
  socks5_udp_send_local_ep: boolean;
  ssrf_mitigation: boolean;
  stop_tracker_timeout: number;
  strict_end_game_mode: boolean;
  suggest_mode: number;
  support_share_mode: boolean;
  tick_interval: number;
  torrent_connect_boost: number;
  tracker_backoff: number;
  tracker_completion_timeout: number;
  tracker_maximum_response_length: number;
  tracker_receive_timeout: number;
  udp_tracker_token_expiry: number;
  unchoke_interval: number;
  unchoke_slots_limit: number;
  upload_rate_limit: number;
  upnp_ignore_nonrouters: boolean;
  upnp_lease_duration: number;
  urlseed_max_request_bytes: number;
  urlseed_pipeline_size: number;
  urlseed_timeout: number;
  urlseed_wait_retry: number;
  use_dht_as_fallback: boolean;
  use_parole_mode: boolean;
  user_agent: string;
  utp_connect_timeout: number;
  utp_cwnd_reduce_timer: number;
  utp_fin_resends: number;
  utp_gain_factor: number;
  utp_loss_multiplier: number;
  utp_min_timeout: number;
  utp_num_resends: number;
  utp_syn_resends: number;
  utp_target_delay: number;
  validate_https_trackers: boolean;
  web_seed_name_lookup_retry: number;
  whole_pieces_threshold: number;
}

type ISettings = {
  settings: ISettingsDict;
}

const AdvancedSettings: Array<{ name: keyof ISettingsDict, desc: string }> = [
  { name: 'active_checking', desc: 'The limit of number of simultaneous checking torrents.' },
  { name: 'active_dht_limit', desc: 'The max number of torrents to announce to the DHT.' },
  { name: 'active_downloads', desc: 'How many active downloading torrents the queuing mechanism allows.' },
  { name: 'active_limit', desc: 'The hard limit on the number of active (auto managed) torrents. This limit also applies to slow torrents.' },
  { name: 'active_lsd_limit', desc: 'The max number of torrents to announce to the local network over the local service discovery protocol.' },
  { name: 'active_seeds', desc: 'How many active seeding torrents the queuing mechanism allows.' },
  { name: 'active_tracker_limit', desc: 'The max number of torrents to announce to their trackers' },
  { name: 'aio_threads', desc: 'For some aio back-ends, ``aio_threads`` specifies the number of io-threads to use.' },
  { name: 'allow_i2p_mixed', desc: 'If this is true, i2p torrents are allowed to also get peers from other sources than the tracker, and connect to regular IPs, not providing any anonymization. This may be useful if the user is not interested in the anonymization of i2p, but still wants to be able to connect to i2p peers.' },
  { name: 'allow_idna', desc: 'When disabled, any tracker or web seed with an IDNA hostname (internationalized domain name) is ignored. This is a security precaution to avoid various unicode encoding attacks that might happen at the application level.' },
  { name: 'allow_multiple_connections_per_ip', desc: 'Determines if connections from the same IP address as existing connections should be rejected or not. Rejecting multiple connections from the same IP address will prevent abusive behavior by peers. The logic for determining whether connections are to the same peer is more complicated with this enabled, and more likely to fail in some edge cases. It is not recommended to enable this feature.' },
  { name: 'allowed_enc_level', desc: 'Determines the encryption level of the connections. This setting will adjust which encryption scheme is offered to the other peer, as well as which encryption scheme is selected by the client. See enc_level enum for options' },
  { name: 'allowed_fast_set_size', desc: 'The number of allowed pieces to send to peers that supports the fast extensions' },
  { name: 'always_send_user_agent', desc: 'Always send user-agent in every web seed request. If false, only the first request per http connection will include the user agent' },
  { name: 'announce_crypto_support', desc: 'When this is true, and incoming encrypted connections are enabled, &supportcrypt=1 is included in http tracker announces' },
  { name: 'announce_ip', desc: 'The IP address passed along to trackers as the ``&ip=`` parameter. If left as the default, that parameter is omitted.' },
  { name: 'announce_to_all_tiers', desc: 'If this is set to true, all trackers in the same tier are announced to in parallel. If all trackers in tier 0 fails, all trackers in tier 1 are announced as well. If it\'s set to false, the behavior is as defined by the multi tracker specification.' },
  { name: 'announce_to_all_trackers', desc: 'When this is set to true, one tracker from each tier is announced to. This is the uTorrent behavior. To be compliant with the Multi-tracker specification, set it to false.' },
  { name: 'anonymous_mode', desc: 'When set to true, the client tries to hide its identity to a certain degree.' },
  { name: 'apply_ip_filter_to_trackers', desc: 'Determines whether the IP filter applies to trackers as well as peers. If this is set to false, trackers are exempt from the IP filter (if there is one). If no IP filter is set, this setting is irrelevant.' },
  { name: 'auto_manage_interval', desc: 'The number of seconds between the torrent queue is updated, and rotated.' },
  { name: 'auto_manage_prefer_seeds', desc: 'If true, prefer seeding torrents when determining which torrents to give active slots to. If false, give preference to downloading torrents' },
  { name: 'auto_manage_startup', desc: 'The number of seconds a torrent is considered active after it was started, regardless of upload and download speed. This is so that newly started torrents are not considered inactive until they have a fair chance to start downloading.' },
  { name: 'auto_scrape_interval', desc: 'The number of seconds between scrapes of queued torrents (auto managed and paused torrents). Auto managed torrents that are paused, are scraped regularly in order to keep track of their downloader/seed ratio. This ratio is used to determine which torrents to seed and which to pause.' },
  { name: 'auto_scrape_min_interval', desc: 'The minimum number of seconds between any automatic scrape (regardless of torrent). In case there are a large number of paused auto managed torrents, this puts a limit on how often a scrape request is sent.' },
  { name: 'auto_sequential', desc: 'If this setting is true, torrents with a very high availability of pieces (and seeds) are downloaded sequentially. This is more efficient for the disk I/O. With many seeds, the download order is unlikely to matter anyway' },
  { name: 'ban_web_seeds', desc: 'When true, web seeds sending bad data will be banned' },
  { name: 'checking_mem_usage', desc: 'The number of blocks to keep outstanding at any given time when checking torrents. Higher numbers give faster re-checks but uses more memory. Specified in number of 16 kiB blocks' },
  { name: 'choking_algorithm', desc: 'Specifies which algorithm to use to determine how many peers to unchoke. The unchoking algorithm for downloading torrents is always "tit-for-tat", i.e. the peers we download the fastest from are unchoked.' },
  { name: 'close_file_interval', desc: 'The number of seconds between closing the file opened the longest ago. 0 means to disable the feature. The purpose of this is to periodically close files to trigger the operating system flushing disk cache. Specifically it has been observed to be required on windows to not have the disk cache grow indefinitely. This defaults to 240 seconds on windows, and disabled on other systems.' },
  { name: 'close_redundant_connections', desc: 'Specifies whether libtorrent should close connections where both ends have no utility in keeping the connection open. For instance if both ends have completed their downloads, there\'s no point in keeping it open.' },
  { name: 'connect_seed_every_n_download', desc: 'This setting controls the priority of downloading torrents over seeding or finished torrents when it comes to making peer connections. Peer connections are throttled by the connection_speed and the half-open connection limit. This makes peer connections a limited resource. Torrents that still have pieces to download are prioritized by default, to avoid having many seeding torrents use most of the connection attempts and only give one peer every now and then to the downloading torrent. libtorrent will loop over the downloading torrents to connect a peer each, and every n:th connection attempt, a finished torrent is picked to be allowed to connect to a peer. This setting controls n.' },
  { name: 'connection_speed', desc: 'The number of connection attempts that are made per second. If a number < 0 is specified, it will default to 200 connections per second. If 0 is specified, it means don\'t make outgoing connections at all.' },
  { name: 'connections_limit', desc: 'Sets a global limit on the number of connections opened. The number of connections is set to a hard minimum of at least two per torrent, so if you set a too low connections limit, and open too many torrents, the limit will not be met.' },
  { name: 'connections_slack', desc: 'The number of incoming connections exceeding the connection limit to accept in order to potentially replace existing ones.' },
  { name: 'dht_aggressive_lookups', desc: 'Slightly changes the lookup behavior in terms of how many outstanding requests we keep. Instead of having branch factor be a hard limit, we always keep *branch factor* outstanding requests to the closest nodes. i.e. every time we get results back with closer nodes, we query them right away. It lowers the lookup times at the cost of more outstanding queries.' },
  { name: 'dht_announce_interval', desc: 'the number of seconds between announcing torrents to the distributed hash table (DHT).' },
  { name: 'dht_block_ratelimit', desc: 'The max number of packets per second a DHT node is allowed to send without getting banned.' },
  { name: 'dht_block_timeout', desc: 'The number of seconds a DHT node is banned if it exceeds the rate limit. The rate limit is averaged over 10 seconds to allow for bursts above the limit.' },
  { name: 'dht_bootstrap_nodes', desc: 'A comma-separated list of IP port-pairs. They will be added to the DHT node (if it\'s enabled) as back-up nodes in case we don\'t know of any.' },
  { name: 'dht_enforce_node_id', desc: '' },
  { name: 'dht_extended_routing_table', desc: '' },
  { name: 'dht_ignore_dark_internet', desc: '' },
  { name: 'dht_item_lifetime', desc: '' },
  { name: 'dht_max_dht_items', desc: '' },
  { name: 'dht_max_fail_count', desc: '' },
  { name: 'dht_max_infohashes_sample_count', desc: '' },
  { name: 'dht_max_peers', desc: '' },
  { name: 'dht_max_peers_reply', desc: '' },
  { name: 'dht_max_torrent_search_reply', desc: '' },
  { name: 'dht_max_torrents', desc: '' },
  { name: 'dht_prefer_verified_node_ids', desc: '' },
  { name: 'dht_privacy_lookups', desc: '' },
  { name: 'dht_read_only', desc: '' },
  { name: 'dht_restrict_routing_ips', desc: '' },
  { name: 'dht_restrict_search_ips', desc: '' },
  { name: 'dht_sample_infohashes_interval', desc: '' },
  { name: 'dht_search_branching', desc: '' },
  { name: 'dht_upload_rate_limit', desc: '' },
  { name: 'disable_hash_checks', desc: '' },
  { name: 'disk_io_read_mode', desc: '' },
  { name: 'disk_io_write_mode', desc: '' },
  { name: 'dont_count_slow_torrents', desc: '' },
  { name: 'download_rate_limit', desc: '' },
  { name: 'enable_dht', desc: '' },
  { name: 'enable_incoming_tcp', desc: '' },
  { name: 'enable_incoming_utp', desc: '' },
  { name: 'enable_ip_notifier', desc: '' },
  { name: 'enable_lsd', desc: '' },
  { name: 'enable_natpmp', desc: '' },
  { name: 'enable_outgoing_tcp', desc: '' },
  { name: 'enable_outgoing_utp', desc: '' },
  { name: 'enable_set_file_valid_data', desc: '' },
  { name: 'enable_upnp', desc: '' },
  { name: 'file_pool_size', desc: '' },
  { name: 'handshake_client_version', desc: '' },
  { name: 'handshake_timeout', desc: '' },
  { name: 'hashing_threads', desc: '' },
  { name: 'i2p_hostname', desc: '' },
  { name: 'i2p_port', desc: '' },
  { name: 'in_enc_policy', desc: '' },
  { name: 'inactive_down_rate', desc: '' },
  { name: 'inactive_up_rate', desc: '' },
  { name: 'inactivity_timeout', desc: '' },
  { name: 'incoming_starts_queued_torrents', desc: '' },
  { name: 'initial_picker_threshold', desc: '' },
  { name: 'listen_interfaces', desc: '' },
  { name: 'listen_queue_size', desc: '' },
  { name: 'listen_system_port_fallback', desc: '' },
  { name: 'local_service_announce_interval', desc: '' },
  { name: 'max_allowed_in_request_queue', desc: '' },
  { name: 'max_concurrent_http_announces', desc: '' },
  { name: 'max_failcount', desc: '' },
  { name: 'max_http_recv_buffer_size', desc: '' },
  { name: 'max_metadata_size', desc: '' },
  { name: 'max_out_request_queue', desc: '' },
  { name: 'max_paused_peerlist_size', desc: '' },
  { name: 'max_peer_recv_buffer_size', desc: '' },
  { name: 'max_peerlist_size', desc: '' },
  { name: 'max_pex_peers', desc: '' },
  { name: 'max_piece_count', desc: '' },
  { name: 'max_queued_disk_bytes', desc: '' },
  { name: 'max_rejects', desc: '' },
  { name: 'max_retry_port_bind', desc: '' },
  { name: 'max_suggest_pieces', desc: '' },
  { name: 'max_web_seed_connections', desc: '' },
  { name: 'metadata_token_limit', desc: '' },
  { name: 'min_announce_interval', desc: '' },
  { name: 'min_reconnect_time', desc: '' },
  { name: 'mixed_mode_algorithm', desc: '' },
  { name: 'no_atime_storage', desc: '' },
  { name: 'no_connect_privileged_ports', desc: '' },
  { name: 'no_recheck_incomplete_resume', desc: '' },
  { name: 'num_optimistic_unchoke_slots', desc: '' },
  { name: 'num_outgoing_ports', desc: '' },
  { name: 'num_want', desc: '' },
  { name: 'optimistic_disk_retry', desc: '' },
  { name: 'optimistic_unchoke_interval', desc: '' },
  { name: 'out_enc_policy', desc: '' },
  { name: 'outgoing_interfaces', desc: '' },
  { name: 'outgoing_port', desc: '' },
  { name: 'peer_connect_timeout', desc: '' },
  { name: 'peer_dscp', desc: '' },
  { name: 'peer_fingerprint', desc: '' },
  { name: 'peer_timeout', desc: '' },
  { name: 'peer_turnover', desc: '' },
  { name: 'peer_turnover_cutoff', desc: '' },
  { name: 'peer_turnover_interval', desc: '' },
  { name: 'piece_extent_affinity', desc: '' },
  { name: 'piece_timeout', desc: '' },
  { name: 'predictive_piece_announce', desc: '' },
  { name: 'prefer_rc4', desc: '' },
  { name: 'prefer_udp_trackers', desc: '' },
  { name: 'prioritize_partial_pieces', desc: '' },
  { name: 'proxy_hostname', desc: '' },
  { name: 'proxy_hostnames', desc: '' },
  { name: 'proxy_password', desc: '' },
  { name: 'proxy_peer_connections', desc: '' },
  { name: 'proxy_port', desc: '' },
  { name: 'proxy_tracker_connections', desc: '' },
  { name: 'proxy_type', desc: '' },
  { name: 'proxy_username', desc: '' },
  { name: 'rate_choker_initial_threshold', desc: '' },
  { name: 'rate_limit_ip_overhead', desc: '' },
  { name: 'recv_socket_buffer_size', desc: '' },
  { name: 'report_redundant_bytes', desc: '' },
  { name: 'report_true_downloaded', desc: '' },
  { name: 'report_web_seed_downloads', desc: '' },
  { name: 'request_queue_time', desc: '' },
  { name: 'request_timeout', desc: '' },
  { name: 'resolver_cache_timeout', desc: '' },
  { name: 'seed_choking_algorithm', desc: '' },
  { name: 'seed_time_limit', desc: '' },
  { name: 'seed_time_ratio_limit', desc: '' },
  { name: 'seeding_outgoing_connections', desc: '' },
  { name: 'seeding_piece_quota', desc: '' },
  { name: 'send_buffer_low_watermark', desc: '' },
  { name: 'send_buffer_watermark', desc: '' },
  { name: 'send_buffer_watermark_factor', desc: '' },
  { name: 'send_not_sent_low_watermark', desc: '' },
  { name: 'send_redundant_have', desc: '' },
  { name: 'send_socket_buffer_size', desc: '' },
  { name: 'share_mode_target', desc: '' },
  { name: 'share_ratio_limit', desc: '' },
  { name: 'smooth_connects', desc: '' },
  { name: 'socks5_udp_send_local_ep', desc: '' },
  { name: 'ssrf_mitigation', desc: '' },
  { name: 'stop_tracker_timeout', desc: '' },
  { name: 'strict_end_game_mode', desc: '' },
  { name: 'suggest_mode', desc: '' },
  { name: 'support_share_mode', desc: '' },
  { name: 'tick_interval', desc: '' },
  { name: 'torrent_connect_boost', desc: '' },
  { name: 'tracker_backoff', desc: '' },
  { name: 'tracker_completion_timeout', desc: '' },
  { name: 'tracker_maximum_response_length', desc: '' },
  { name: 'tracker_receive_timeout', desc: '' },
  { name: 'udp_tracker_token_expiry', desc: '' },
  { name: 'unchoke_interval', desc: '' },
  { name: 'unchoke_slots_limit', desc: '' },
  { name: 'upload_rate_limit', desc: '' },
  { name: 'upnp_ignore_nonrouters', desc: '' },
  { name: 'upnp_lease_duration', desc: '' },
  { name: 'urlseed_max_request_bytes', desc: '' },
  { name: 'urlseed_pipeline_size', desc: '' },
  { name: 'urlseed_timeout', desc: '' },
  { name: 'urlseed_wait_retry', desc: '' },
  { name: 'use_dht_as_fallback', desc: '' },
  { name: 'use_parole_mode', desc: '' },
  { name: 'user_agent', desc: '' },
  { name: 'utp_connect_timeout', desc: '' },
  { name: 'utp_cwnd_reduce_timer', desc: '' },
  { name: 'utp_fin_resends', desc: '' },
  { name: 'utp_gain_factor', desc: '' },
  { name: 'utp_loss_multiplier', desc: '' },
  { name: 'utp_min_timeout', desc: '' },
  { name: 'utp_num_resends', desc: '' },
  { name: 'utp_syn_resends', desc: '' },
  { name: 'utp_target_delay', desc: '' },
  { name: 'validate_https_trackers', desc: '' },
  { name: 'web_seed_name_lookup_retry', desc: '' },
  { name: 'whole_pieces_threshold', desc: '' },
]

function settingInput(settings: ISettingsDict, name: keyof ISettingsDict) {
  if (typeof settings[name] === 'boolean') {
    return <Checkbox checked={settings[name] as boolean} readOnly />
  }

  if (typeof settings[name] === 'string') {
    return <Input size={'sm'} type={'text'} readOnly value={settings[name] as string} />
  }

  return <Input size={'sm'} type={'number'} readOnly value={settings[name] as number} />
}

export default function Settings() {
  const {data,error} = jsonrpc<ISettings>('session.settings.list');

  if (!data) {
    return <div>Loading...</div>
  }

  return (
    <Box m={3}>
      <Tabs variant={'line'}>
        <TabList>
          <Tab>Libtorrent settings</Tab>
        </TabList>
        <TabPanels>
          <TabPanel>
            <Table size={'sm'}>
              <Thead>
                <Tr>
                  <Th width={'30%'}>Name</Th>
                  <Th width={'20%'}>Value</Th>
                  <Th width={'50%'}>Description</Th>
                </Tr>
              </Thead>
              <Tbody>
                { AdvancedSettings.map(as => (
                  <Tr key={as.name}>
                    <Td>{as.name}</Td>
                    <Td>
                      {settingInput(data.settings, as.name)}
                    </Td>
                    <Td>{as.desc}</Td>
                  </Tr>
                ))}
              </Tbody>
            </Table>
          </TabPanel>
        </TabPanels>
      </Tabs>
    </Box>
  )
}
