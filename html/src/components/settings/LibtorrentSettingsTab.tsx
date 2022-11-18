import { Kbd, Checkbox, Input, Link, Text, FormControl, FormLabel, FormHelperText, Divider, Box } from "@chakra-ui/react";
import { FastField, useFormikContext } from "formik";
import { ISettingsDict } from "../../services";
import EncPolicyInput from "./EncPolicyInput";
import IoBufferModeInput from "./IoBufferModeInput";

type AdvancedSetting = {
  name: keyof ISettingsDict;
  desc: string | JSX.Element;
  el?: (data: boolean | number | string) => JSX.Element;
}

const AdvancedSettings: Array<AdvancedSetting> = [
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
  { name: 'dht_enforce_node_id', desc: 'When set, node\'s whose IDs that are not correctly generated based on its external IP are ignored. When a query arrives from such node, an error message is returned with a message saying "invalid node ID".' },
  { name: 'dht_extended_routing_table', desc: 'Makes the first buckets in the DHT routing table fit 128, 64, 32 and 16 nodes respectively, as opposed to the standard size of 8. All other buckets have size 8 still.' },
  { name: 'dht_ignore_dark_internet', desc: 'Ignore DHT messages from parts of the internet we wouldn\'t expect to see any traffic from' },
  { name: 'dht_item_lifetime', desc: 'The number of seconds a immutable/mutable item will be expired. Default is 0, means never expires.' },
  { name: 'dht_max_dht_items', desc: 'Max number of items the DHT will store' },
  { name: 'dht_max_fail_count', desc: 'The maximum number of failed tries to contact a node before it is removed from the routing table. If there are known working nodes that are ready to replace a failing node, it will be replaced immediately, this limit is only used to clear out nodes that don\'t have any node that can replace them.' },
  { name: 'dht_max_infohashes_sample_count', desc: 'The maximum number of elements in the sampled subset of info-hashes. If this number is too big, expect the DHT storage implementations to clamp it in order to allow UDP packets go through' },
  { name: 'dht_max_peers', desc: 'The max number of peers to store per torrent (for the DHT)' },
  { name: 'dht_max_peers_reply', desc: 'The maximum number of peers to send in a reply to ``get_peers``' },
  { name: 'dht_max_torrent_search_reply', desc: 'The max number of torrents to return in a torrent search query to the DHT' },
  { name: 'dht_max_torrents', desc: 'The total number of torrents to track from the DHT. This is simply an upper limit to make sure malicious DHT nodes cannot make us allocate an unbounded amount of memory.' },
  { name: 'dht_prefer_verified_node_ids', desc: 'When this is true, nodes whose IDs are derived from their source IP according to `BEP 42`_ are preferred in the routing table.' },
  { name: 'dht_privacy_lookups', desc: 'When set, perform lookups in a way that is slightly more expensive, but which minimizes the amount of information leaked about you.' },
  { name: 'dht_read_only', desc: 'When set, the other nodes won\'t keep this node in their routing tables, it\'s meant for low-power and/or ephemeral devices that cannot support the DHT, it is also useful for mobile devices which are sensitive to network traffic and battery life. this node no longer responds to \'query\' messages, and will place a \'ro\' key (value = 1) in the top-level message dictionary of outgoing query messages.' },
  { name: 'dht_restrict_routing_ips', desc: 'Determines if the routing table entries should restrict entries to one per IP. This defaults to true, which helps mitigate some attacks on the DHT. It prevents adding multiple nodes with IPs with a very close CIDR distance.' },
  { name: 'dht_restrict_search_ips', desc: 'Determines if DHT searches should prevent adding nodes with IPs with very close CIDR distance. This also defaults to true and helps mitigate certain attacks on the DHT.' },
  { name: 'dht_sample_infohashes_interval', desc: 'The info-hashes sample recomputation interval (in seconds). The node will precompute a subset of the tracked info-hashes and return that instead of calculating it upon each request. The permissible range is between 0 and 21600 seconds (inclusive).' },
  { name: 'dht_search_branching', desc: 'The number of concurrent search request the node will send when announcing and refreshing the routing table. This parameter is called alpha in the kademlia paper' },
  { name: 'dht_upload_rate_limit', desc: 'The number of bytes per second (on average) the DHT is allowed to send. If the incoming requests causes to many bytes to be sent in responses, incoming requests will be dropped until the quota has been replenished.' },
  { name: 'disable_hash_checks', desc: 'When set to true, all data downloaded from peers will be assumed to be correct, and not tested to match the hashes in the torrent this is only useful for simulation and testing purposes (typically combined with disabled_storage)' },
  { name: 'disk_io_read_mode', desc: 'Determines how files are opened when they\'re in read only mode', el: (val: boolean | number | string) => <IoBufferModeInput value={val as number} /> },
  { name: 'disk_io_write_mode', desc: 'Determines how files are opened when they\'re in read and write mode', el: (val: boolean | number | string) => <IoBufferModeInput value={val as number} /> },
  { name: 'dont_count_slow_torrents', desc: 'If true, torrents without any payload transfers are not subject to the ``active_seeds`` and ``active_downloads`` limits. This is intended to make it more likely to utilize all available bandwidth, and avoid having torrents that don\'t transfer anything block the active slots.' },
  { name: 'download_rate_limit', desc: 'Sets the session-global limits of download rate limits, in bytes per second. By default peers on the local network are not rate limited.' },
  { name: 'enable_dht', desc: 'If true, starts the dht node and makes the trackerless service available to torrents.' },
  { name: 'enable_incoming_tcp', desc: '' },
  { name: 'enable_incoming_utp', desc: '' },
  { name: 'enable_ip_notifier', desc: 'Starts and stops the internal IP table route changes notifier. The current implementation supports multiple platforms, and it is recommended to have it enable, but you may want to disable it if it\'s supported but unreliable, or if you have a better way to detect the changes.' },
  { name: 'enable_lsd', desc: 'Starts and stops Local Service Discovery. This service will broadcast the info-hashes of all the non-private torrents on the local network to look for peers on the same swarm within multicast reach.' },
  { name: 'enable_natpmp', desc: 'Starts and stops the NAT-PMP service. When started, the listen port and the DHT port are attempted to be forwarded on the router through NAT-PMP.' },
  { name: 'enable_outgoing_tcp', desc: '' },
  { name: 'enable_outgoing_utp', desc: '' },
  { name: 'enable_set_file_valid_data', desc: 'When set to true, enables the attempt to use SetFileValidData() to pre-allocate disk space. This system call will only work when running with Administrator privileges on Windows, and so this setting is only relevant in that scenario. Using SetFileValidData() poses a security risk, as it may reveal previously deleted information from the disk.' },
  { name: 'enable_upnp', desc: 'Starts and stops the UPnP service. When started, the listen port and the DHT port are attempted to be forwarded on local UPnP router devices.' },
  { name: 'file_pool_size', desc: 'Sets the upper limit on the total number of files this session will keep open. The reason why files are left open at all is that some anti virus software hooks on every file close, and scans the file for viruses. deferring the closing of the files will be the difference between a usable system and a completely hogged down system. Most operating systems also has a limit on the total number of file descriptors a process may have open.' },
  { name: 'handshake_client_version', desc: 'This is the client name and version identifier sent to peers in the handshake message. If this is an empty string, the user_agent is used instead. This string must be a UTF-8 encoded unicode string.' },
  { name: 'handshake_timeout', desc: 'The number of seconds to wait for a handshake response from a peer. If no response is received within this time, the peer is disconnected.' },
  { name: 'hashing_threads',
    desc: <><p>The number of disk I/O threads to use for
            piece hash verification. These threads are *in addition* to the
            regular disk I/O threads specified by <Kbd>aio_threads</Kbd>.
            These threads are only used for full checking of torrents. The
            hash checking done while downloading are done by the regular disk
            I/O threads.</p>
            <Text mt={2}>The hasher threads do not only compute hashes, but also perform
            the read from disk. On storage optimal for sequential access,
            such as hard drives, this setting should be set to 1, which is
            also the default.</Text></>
  },
  { name: 'i2p_hostname',
    desc: <>Sets the <Link href="http://i2p2.de" isExternal>i2p</Link> SAM bridge to connect to.</>
  },
  { name: 'i2p_port',
    desc: <>Sets the <Link href="http://i2p2.de" isExternal>i2p</Link> SAM bridge port to connect to.</>
  },
  { name: 'in_enc_policy',
    desc:
    <>
    Controls the settings for incoming connections. Keep in mind that protocol encryption degrades performance in
		several respects:
		<ol>
      <li>It prevents "zero copy" disk buffers being sent to peers, since
		   each peer needs to mutate the data (i.e. encrypt it) the data
		   must be copied per peer connection rather than sending the same
		   buffer to multiple peers.</li>
       <li>The encryption itself requires more CPU than plain bittorrent
		   protocol. The highest cost is the Diffie Hellman exchange on
		   connection setup.</li>
       <li>The encryption handshake adds several round-trips to the
		   connection setup, and delays transferring data.</li>
    </ol>
    </>,
    el: (val: boolean | number | string) => <EncPolicyInput value={val as number} />
  },
  { name: 'inactive_down_rate', desc: 'The download rate limit for a torrent to be considered active by the queuing mechanism. A torrent whose download rate is less than ``inactive_down_rate`` and whose upload rate is less than ``inactive_up_rate`` for ``auto_manage_startup`` seconds, is considered inactive, and another queued torrent may be started. This logic is disabled if ``dont_count_slow_torrents`` is false.' },
  { name: 'inactive_up_rate', desc: 'The upload rate limit for a torrent to be considered active by the queuing mechanism. A torrent whose download rate is less than ``inactive_down_rate`` and whose upload rate is less than ``inactive_up_rate`` for ``auto_manage_startup`` seconds, is considered inactive, and another queued torrent may be started. This logic is disabled if ``dont_count_slow_torrents`` is false.' },
  { name: 'inactivity_timeout', desc: 'If a peer is uninteresting and uninterested for longer than this number of seconds, it will be disconnected.' },
  { name: 'incoming_starts_queued_torrents', desc: 'If a torrent has been paused by the auto managed feature in libtorrent, i.e. the torrent is paused and auto managed, this feature affects whether or not it is automatically started on an incoming connection. The main reason to queue torrents, is not to make them unavailable, but to save on the overhead of announcing to the trackers, the DHT and to avoid spreading one\'s unchoke slots too thin. If a peer managed to find us, even though we\'re no in the torrent anymore, this setting can make us start the torrent and serve it.' },
  { name: 'initial_picker_threshold', desc: 'Specifies the number of pieces we need before we switch to rarest first picking. The first ``initial_picker_threshold`` pieces in any torrent are picked at random , the following pieces are picked in rarest first order.' },
  { name: 'listen_interfaces', desc: 'A comma-separated list of (IP or device name, port) pairs. These are the listen ports that will be opened for accepting incoming uTP and TCP peer connections. These are also used for *outgoing* uTP and UDP tracker connections and DHT nodes.' },
  { name: 'listen_queue_size', desc: 'The number of outstanding incoming connections to queue up while we\'re not actively waiting for a connection to be accepted. 5 should be sufficient for any normal client. If this is a high performance server which expects to receive a lot of connections, or used in a simulator or test, it might make sense to raise this number. It will not take affect until the ``listen_interfaces`` settings is updated.' },
  { name: 'listen_system_port_fallback', desc: 'If this is true, libtorrent will fall back to listening on a port chosen by the operating system (i.e. binding to port 0). If a failure is preferred, set this to false.' },
  { name: 'local_service_announce_interval', desc: 'The time in seconds between local network announces for a torrent.' },
  { name: 'max_allowed_in_request_queue', desc: 'The number of outstanding block requests a peer is allowed to queue up in the client. If a peer sends more requests than this (before the first one has been sent) the last request will be dropped. the higher this is, the faster upload speeds the client can get to a single peer.' },
  { name: 'max_concurrent_http_announces', desc: 'Limits the number of concurrent HTTP tracker announces. Once the limit is hit, tracker requests are queued and issued when an outstanding announce completes.' },
  { name: 'max_failcount', desc: 'The maximum times we try to connect to a peer before stop connecting again. If a peer succeeds, the failure counter is reset. If a peer is retrieved from a peer source (other than DHT) the failcount is decremented by one, allowing another try.' },
  { name: 'max_http_recv_buffer_size', desc: 'The max number of bytes to allow an HTTP response to be when announcing to trackers or downloading .torrent files via the ``url`` provided in ``add_torrent_params``.' },
  { name: 'max_metadata_size', desc: 'The maximum allowed size (in bytes) to be received by the metadata extension, i.e. magnet links.' },
  { name: 'max_out_request_queue', desc: 'The maximum number of outstanding requests to send to a peer. This limit takes precedence over ``request_queue_time``. i.e. no matter the download speed, the number of outstanding requests will never exceed this limit.' },
  { name: 'max_paused_peerlist_size', desc: 'The max peer list size used for torrents that are paused. This can be used to save memory for paused torrents, since it\'s not as important for them to keep a large peer list.' },
  { name: 'max_peer_recv_buffer_size', desc: 'The max number of bytes a single peer connection\'s receive buffer is allowed to grow to.' },
  { name: 'max_peerlist_size', desc: 'The maximum number of peers in the list of known peers. These peers are not necessarily connected, so this number should be much greater than the maximum number of connected peers. Peers are evicted from the cache when the list grows passed 90% of this limit, and once the size hits the limit, peers are no longer added to the list. If this limit is set to 0, there is no limit on how many peers we\'ll keep in the peer list.' },
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

function settingInput(w: any, settings: ISettingsDict, name: keyof ISettingsDict) {
  if (typeof settings[name] === 'boolean') {
    return <Checkbox {...w.field} />
  }

  if (typeof settings[name] === 'string') {
    return <Input {...w.field} type={"text"} />
  }

  return <Input {...w.field} type={"number"} />
}

type LibtorrentSettingsTabProps = {
  settings: ISettingsDict;
}

export default function LibtorrentSettingsTab(props: LibtorrentSettingsTabProps) {
  const {
    errors,
    handleChange,
    setFieldValue,
    touched,
    values
  } = useFormikContext<ISettingsDict>();

  return (
    <>
      { AdvancedSettings.map(as => (
        <Box key={as.name}>
          <FastField name={as.name}>
            {(w: any) => (
              <FormControl mb={3}>
                <FormLabel>{as.name}</FormLabel>
                {
                  as.el
                    ? as.el(props.settings[as.name])
                    : settingInput(w, props.settings, as.name)
                }
                <FormHelperText>{as.desc}</FormHelperText>
              </FormControl>
            )}
          </FastField>
          <Divider mb={3} />
        </Box>
      ))}
    </>
  )
}
