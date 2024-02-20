type AddTorrentParams = {
  save_path: string;
  ti?: string;
}

export type PresetsList = {
  [id: string] : Preset;
};

export type Preset = {
  save_path: string | null
};

export type Torrent = {
  all_time_download: number;
  all_time_upload: number;
  category: string | null;
  download_rate: number;
  download_limit: number;
  num_peers: number;
  num_seeds: number;
  num_complete: number;
  num_incomplete: number;
  list_peers: number;
  list_seeds: number;
  error: any;
  eta: number;
  flags: string[];
  info_hash: InfoHash;
  moving_storage: boolean;
  name: string;
  progress: number;
  queue_position: number;
  save_path: string;
  session: string;
  ratio: number;
  size: number;
  state: number;
  tags: string[];
  upload_rate: number;
}

export type TorrentPropertiesFlags = {
  auto_managed?: boolean;
  sequential_download?: boolean;
}

export type TorrentProperties = {
  download_limit: number;
  flags: TorrentPropertiesFlags;
  max_connections: number;
  max_uploads: number;
  upload_limit: number;
}

export type ITorrentsList = {
  order_by: string;
  order_by_dir: string;
  page_size: number;
  torrents: Torrent[];
  torrents_total: number;
  torrents_total_unfiltered: number;
};

export type InfoHash = [string | null, string | null];

export type ISettingsDict = {
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

export type ISettingsList = {
  settings: ISettingsDict;
}

export type TorrentsListFilters = {
  category?: string | null | undefined;
  query?: string | null | undefined;
  save_path?: string | null | undefined;
  tags?: string[] | null | undefined;
}
