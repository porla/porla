#include "settingspack.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/fingerprint.hpp>
#include <libtorrent/session.hpp>

namespace lt = libtorrent;
using porla::SettingsPack;

lt::settings_pack SettingsPack::Load(const toml::table &cfg)
{
    const std::string base = cfg["libtorrent"]["base"].value_or("default");

    auto pack = base == "min_memory_usage"
        ? lt::min_memory_usage()
        : base == "high_performance_seed"
            ? lt::high_performance_seed()
            : lt::default_settings();

    // Apply default settings here. These can be overwritten by settings from the
    // config file.
    pack.set_str(lt::settings_pack::user_agent, "porla/1.0");

    for (auto const& [setting, key] : AllowedStringSettings())
    {
        if (auto val = cfg["libtorrent"][setting])
        {
            if (!val.is_string())
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "Invalid setting libtorrent."
                    << setting << " - not a string (" << val.type() << ")";

                continue;
            }

            pack.set_str(key, *val.value<std::string>());
        }
    }

    for (auto const& [setting, key] : AllowedBoolSettings())
    {
        if (auto val = cfg["libtorrent"][setting])
        {
            if (!val.is_boolean())
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "Invalid setting libtorrent."
                    << setting << " - not a bool (" << val.type() << ")";

                continue;
            }

            pack.set_bool(key, *val.value<bool>());
        }
    }

    for (auto const& [setting, key] : AllowedIntSettings())
    {
        if (auto val = cfg["libtorrent"][setting])
        {
            if (!val.is_number())
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "Invalid setting libtorrent."
                    << setting << " - not a number (" << val.type() << ")";

                continue;
            }

            pack.set_int(key, *val.value<int>());
        }
    }

    // Apply static settings here. These are always set after all other settings from
    // the config are applied, and cannot be overwritten by it.
    pack.set_str(lt::settings_pack::peer_fingerprint, lt::generate_fingerprint("PO", 0, 1));

    if (cfg.contains("proxy"))
    {
        if (auto host = cfg["proxy"]["host"].value<std::string>())
            pack.set_str(lt::settings_pack::proxy_hostname, *host);

        if (auto port = cfg["proxy"]["port"].value<int>())
            pack.set_int(lt::settings_pack::proxy_port, *port);

        if (auto type = cfg["proxy"]["type"].value<std::string>())
        {
            lt::settings_pack::proxy_type_t ltType = lt::settings_pack::none;

            if (type == "socks4")    ltType = lt::settings_pack::socks4;
            if (type == "socks5")    ltType = lt::settings_pack::socks5;
            if (type == "socks5_pw") ltType = lt::settings_pack::socks5_pw;
            if (type == "http")      ltType = lt::settings_pack::http;
            if (type == "http_pw")   ltType = lt::settings_pack::http_pw;
            if (type == "i2p_proxy") ltType = lt::settings_pack::i2p_proxy;

            pack.set_int(lt::settings_pack::proxy_type, ltType);
        }

        if (auto user = cfg["proxy"]["username"].value<std::string>())
            pack.set_str(lt::settings_pack::proxy_username, *user);

        if (auto pwd = cfg["proxy"]["password"].value<std::string>())
            pack.set_str(lt::settings_pack::proxy_password, *pwd);
    }

    return pack;
}

std::map<std::string, int> SettingsPack::AllowedBoolSettings()
{
    static std::map<std::string, int> bool_settings =
    {
        {"allow_multiple_connections_per_ip", lt::settings_pack::allow_multiple_connections_per_ip},
        {"send_redundant_have", lt::settings_pack::send_redundant_have},
        {"use_dht_as_fallback", lt::settings_pack::use_dht_as_fallback},
        {"upnp_ignore_nonrouters", lt::settings_pack::upnp_ignore_nonrouters},
        {"use_parole_mode", lt::settings_pack::use_parole_mode},
        {"auto_manage_prefer_seeds", lt::settings_pack::auto_manage_prefer_seeds},
        {"dont_count_slow_torrents", lt::settings_pack::dont_count_slow_torrents},
        {"close_redundant_connections", lt::settings_pack::close_redundant_connections},
        {"prioritize_partial_pieces", lt::settings_pack::prioritize_partial_pieces},
        {"rate_limit_ip_overhead", lt::settings_pack::rate_limit_ip_overhead},
        {"announce_to_all_tiers", lt::settings_pack::announce_to_all_tiers},
        {"announce_to_all_trackers", lt::settings_pack::announce_to_all_trackers},
        {"prefer_udp_trackers", lt::settings_pack::prefer_udp_trackers},
        {"disable_hash_checks", lt::settings_pack::disable_hash_checks},
        {"allow_i2p_mixed", lt::settings_pack::allow_i2p_mixed},
        {"no_atime_storage", lt::settings_pack::no_atime_storage},
        {"incoming_starts_queued_torrents", lt::settings_pack::incoming_starts_queued_torrents},
        {"report_true_downloaded", lt::settings_pack::report_true_downloaded},
        {"strict_end_game_mode", lt::settings_pack::strict_end_game_mode},
        {"strict_end_game_mode", lt::settings_pack::strict_end_game_mode},
        {"enable_outgoing_utp", lt::settings_pack::enable_outgoing_utp},
        {"enable_incoming_utp", lt::settings_pack::enable_incoming_utp},
        {"enable_outgoing_tcp", lt::settings_pack::enable_outgoing_tcp},
        {"enable_incoming_tcp", lt::settings_pack::enable_incoming_tcp},
        {"no_recheck_incomplete_resume", lt::settings_pack::no_recheck_incomplete_resume},
        {"anonymous_mode", lt::settings_pack::anonymous_mode},
        {"report_web_seed_downloads", lt::settings_pack::report_web_seed_downloads},
        {"seeding_outgoing_connections", lt::settings_pack::seeding_outgoing_connections},
        {"no_connect_privileged_ports", lt::settings_pack::no_connect_privileged_ports},
        {"smooth_connects", lt::settings_pack::smooth_connects},
        {"always_send_user_agent", lt::settings_pack::always_send_user_agent},
        {"apply_ip_filter_to_trackers", lt::settings_pack::apply_ip_filter_to_trackers},
        {"ban_web_seeds", lt::settings_pack::ban_web_seeds},
        {"support_share_mode", lt::settings_pack::support_share_mode},
        {"report_redundant_bytes", lt::settings_pack::report_redundant_bytes},
        {"listen_system_port_fallback", lt::settings_pack::listen_system_port_fallback},
        {"announce_crypto_support", lt::settings_pack::announce_crypto_support},
        {"enable_upnp", lt::settings_pack::enable_upnp},
        {"enable_natpmp", lt::settings_pack::enable_natpmp},
        {"enable_lsd", lt::settings_pack::enable_lsd},
        {"enable_dht", lt::settings_pack::enable_dht},
        {"prefer_rc4", lt::settings_pack::prefer_rc4},
        {"auto_sequential", lt::settings_pack::auto_sequential},
        {"enable_ip_notifier", lt::settings_pack::enable_ip_notifier},
        {"dht_prefer_verified_node_ids", lt::settings_pack::dht_prefer_verified_node_ids},
        {"dht_restrict_routing_ips", lt::settings_pack::dht_restrict_routing_ips},
        {"dht_restrict_search_ips", lt::settings_pack::dht_restrict_search_ips},
        {"dht_extended_routing_table", lt::settings_pack::dht_extended_routing_table},
        {"dht_aggressive_lookups", lt::settings_pack::dht_aggressive_lookups},
        {"dht_privacy_lookups", lt::settings_pack::dht_privacy_lookups},
        {"dht_enforce_node_id", lt::settings_pack::dht_enforce_node_id},
        {"dht_ignore_dark_internet", lt::settings_pack::dht_ignore_dark_internet},
        {"dht_read_only", lt::settings_pack::dht_read_only},
        {"piece_extent_affinity", lt::settings_pack::piece_extent_affinity},
        {"validate_https_trackers", lt::settings_pack::validate_https_trackers},
        {"ssrf_mitigation", lt::settings_pack::ssrf_mitigation},
        {"allow_idna", lt::settings_pack::allow_idna},
        {"enable_set_file_valid_data", lt::settings_pack::enable_set_file_valid_data},
        {"socks5_udp_send_local_ep", lt::settings_pack::socks5_udp_send_local_ep},
    };

    return bool_settings;
}

std::map<std::string, int> SettingsPack::AllowedIntSettings()
{
    static std::map<std::string, int> int_settings =
    {
        {"tracker_completion_timeout", lt::settings_pack::tracker_completion_timeout},
        {"tracker_receive_timeout", lt::settings_pack::tracker_receive_timeout},
        {"stop_tracker_timeout", lt::settings_pack::stop_tracker_timeout},
        {"tracker_maximum_response_length", lt::settings_pack::tracker_maximum_response_length},
        {"piece_timeout", lt::settings_pack::piece_timeout},
        {"request_timeout", lt::settings_pack::request_timeout},
        {"request_queue_time", lt::settings_pack::request_queue_time},
        {"max_allowed_in_request_queue", lt::settings_pack::max_allowed_in_request_queue},
        {"max_out_request_queue", lt::settings_pack::max_out_request_queue},
        {"whole_pieces_threshold", lt::settings_pack::whole_pieces_threshold},
        {"peer_timeout", lt::settings_pack::peer_timeout},
        {"urlseed_timeout", lt::settings_pack::urlseed_timeout},
        {"urlseed_pipeline_size", lt::settings_pack::urlseed_pipeline_size},
        {"urlseed_wait_retry", lt::settings_pack::urlseed_wait_retry},
        {"file_pool_size", lt::settings_pack::file_pool_size},
        {"max_failcount", lt::settings_pack::max_failcount},
        {"min_reconnect_time", lt::settings_pack::min_reconnect_time},
        {"peer_connect_timeout", lt::settings_pack::peer_connect_timeout},
        {"connection_speed", lt::settings_pack::connection_speed},
        {"inactivity_timeout", lt::settings_pack::inactivity_timeout},
        {"unchoke_interval", lt::settings_pack::unchoke_interval},
        {"optimistic_unchoke_interval", lt::settings_pack::optimistic_unchoke_interval},
        {"num_want", lt::settings_pack::num_want},
        {"initial_picker_threshold", lt::settings_pack::initial_picker_threshold},
        {"allowed_fast_set_size", lt::settings_pack::allowed_fast_set_size},
        {"suggest_mode", lt::settings_pack::suggest_mode},
        {"max_queued_disk_bytes", lt::settings_pack::max_queued_disk_bytes},
        {"handshake_timeout", lt::settings_pack::handshake_timeout},
        {"handshake_timeout", lt::settings_pack::handshake_timeout},
        {"send_buffer_low_watermark", lt::settings_pack::send_buffer_low_watermark},
        {"send_buffer_watermark", lt::settings_pack::send_buffer_watermark},
        {"send_buffer_watermark_factor", lt::settings_pack::send_buffer_watermark_factor},
        {"choking_algorithm", lt::settings_pack::choking_algorithm},
        {"seed_choking_algorithm", lt::settings_pack::seed_choking_algorithm},
        {"disk_io_write_mode", lt::settings_pack::disk_io_write_mode},
        {"disk_io_read_mode", lt::settings_pack::disk_io_read_mode},
        {"active_downloads", lt::settings_pack::active_downloads},
        {"active_seeds", lt::settings_pack::active_seeds},
        {"active_checking", lt::settings_pack::active_checking},
        {"active_dht_limit", lt::settings_pack::active_dht_limit},
        {"active_tracker_limit", lt::settings_pack::active_tracker_limit},
        {"active_lsd_limit", lt::settings_pack::active_lsd_limit},
        {"active_limit", lt::settings_pack::active_limit},
        {"auto_manage_interval", lt::settings_pack::auto_manage_interval},
        {"seed_time_limit", lt::settings_pack::seed_time_limit},
        {"auto_scrape_interval", lt::settings_pack::auto_scrape_interval},
        {"auto_scrape_min_interval", lt::settings_pack::auto_scrape_min_interval},
        {"max_peerlist_size", lt::settings_pack::max_peerlist_size},
        {"max_paused_peerlist_size", lt::settings_pack::max_paused_peerlist_size},
        {"min_announce_interval", lt::settings_pack::min_announce_interval},
        {"auto_manage_startup", lt::settings_pack::auto_manage_startup},
        {"seeding_piece_quota", lt::settings_pack::seeding_piece_quota},
        {"max_rejects", lt::settings_pack::max_rejects},
        {"recv_socket_buffer_size", lt::settings_pack::recv_socket_buffer_size},
        {"send_socket_buffer_size", lt::settings_pack::send_socket_buffer_size},
        {"max_peer_recv_buffer_size", lt::settings_pack::max_peer_recv_buffer_size},
        {"optimistic_disk_retry", lt::settings_pack::optimistic_disk_retry},
        {"max_suggest_pieces", lt::settings_pack::max_suggest_pieces},
        {"local_service_announce_interval", lt::settings_pack::local_service_announce_interval},
        {"dht_announce_interval", lt::settings_pack::dht_announce_interval},
        {"udp_tracker_token_expiry", lt::settings_pack::udp_tracker_token_expiry},
        {"num_optimistic_unchoke_slots", lt::settings_pack::num_optimistic_unchoke_slots},
        {"max_pex_peers", lt::settings_pack::max_pex_peers},
        {"tick_interval", lt::settings_pack::tick_interval},
        {"share_mode_target", lt::settings_pack::share_mode_target},
        {"upload_rate_limit", lt::settings_pack::upload_rate_limit},
        {"download_rate_limit", lt::settings_pack::download_rate_limit},
        {"dht_upload_rate_limit", lt::settings_pack::dht_upload_rate_limit},
        {"unchoke_slots_limit", lt::settings_pack::unchoke_slots_limit},
        {"connections_limit", lt::settings_pack::connections_limit},
        {"connections_slack", lt::settings_pack::connections_slack},
        {"utp_target_delay", lt::settings_pack::utp_target_delay},
        {"utp_gain_factor", lt::settings_pack::utp_gain_factor},
        {"utp_min_timeout", lt::settings_pack::utp_min_timeout},
        {"utp_syn_resends", lt::settings_pack::utp_syn_resends},
        {"utp_fin_resends", lt::settings_pack::utp_fin_resends},
        {"utp_num_resends", lt::settings_pack::utp_num_resends},
        {"utp_connect_timeout", lt::settings_pack::utp_connect_timeout},
        {"utp_loss_multiplier", lt::settings_pack::utp_loss_multiplier},
        {"mixed_mode_algorithm", lt::settings_pack::mixed_mode_algorithm},
        {"listen_queue_size", lt::settings_pack::listen_queue_size},
        {"torrent_connect_boost", lt::settings_pack::torrent_connect_boost},
        {"alert_queue_size", lt::settings_pack::alert_queue_size},
        {"max_metadata_size", lt::settings_pack::max_metadata_size},
        {"hashing_threads", lt::settings_pack::hashing_threads},
        {"checking_mem_usage", lt::settings_pack::checking_mem_usage},
        {"predictive_piece_announce", lt::settings_pack::predictive_piece_announce},
        {"aio_threads", lt::settings_pack::aio_threads},
        {"tracker_backoff", lt::settings_pack::tracker_backoff},
        {"share_ratio_limit", lt::settings_pack::share_ratio_limit},
        {"seed_time_ratio_limit", lt::settings_pack::seed_time_ratio_limit},
        {"peer_turnover", lt::settings_pack::peer_turnover},
        {"peer_turnover_cutoff", lt::settings_pack::peer_turnover_cutoff},
        {"peer_turnover_interval", lt::settings_pack::peer_turnover_interval},
        {"connect_seed_every_n_download", lt::settings_pack::connect_seed_every_n_download},
        {"max_http_recv_buffer_size", lt::settings_pack::max_http_recv_buffer_size},
        {"max_retry_port_bind", lt::settings_pack::max_retry_port_bind},
        {"out_enc_policy", lt::settings_pack::out_enc_policy},
        {"in_enc_policy", lt::settings_pack::in_enc_policy},
        {"allowed_enc_level", lt::settings_pack::allowed_enc_level},
        {"inactive_down_rate", lt::settings_pack::inactive_down_rate},
        {"inactive_up_rate", lt::settings_pack::inactive_up_rate},
        {"urlseed_max_request_bytes", lt::settings_pack::urlseed_max_request_bytes},
        {"web_seed_name_lookup_retry", lt::settings_pack::web_seed_name_lookup_retry},
        {"close_file_interval", lt::settings_pack::close_file_interval},
        {"utp_cwnd_reduce_timer", lt::settings_pack::utp_cwnd_reduce_timer},
        {"max_web_seed_connections", lt::settings_pack::max_web_seed_connections},
        {"resolver_cache_timeout", lt::settings_pack::resolver_cache_timeout},
        {"send_not_sent_low_watermark", lt::settings_pack::send_not_sent_low_watermark},
        {"rate_choker_initial_threshold", lt::settings_pack::rate_choker_initial_threshold},
        {"upnp_lease_duration", lt::settings_pack::upnp_lease_duration},
        {"max_concurrent_http_announces", lt::settings_pack::max_concurrent_http_announces},
        {"dht_max_peers_reply", lt::settings_pack::dht_max_peers_reply},
        {"dht_search_branching", lt::settings_pack::dht_search_branching},
        {"dht_max_fail_count", lt::settings_pack::dht_max_fail_count},
        {"dht_max_torrents", lt::settings_pack::dht_max_torrents},
        {"dht_max_dht_items", lt::settings_pack::dht_max_dht_items},
        {"dht_max_peers", lt::settings_pack::dht_max_peers},
        {"dht_max_torrent_search_reply", lt::settings_pack::dht_max_torrent_search_reply},
        {"dht_block_timeout", lt::settings_pack::dht_block_timeout},
        {"dht_block_ratelimit", lt::settings_pack::dht_block_ratelimit},
        {"dht_item_lifetime", lt::settings_pack::dht_item_lifetime},
        {"dht_sample_infohashes_interval", lt::settings_pack::dht_sample_infohashes_interval},
        {"dht_max_infohashes_sample_count", lt::settings_pack::dht_max_infohashes_sample_count},
        {"max_piece_count", lt::settings_pack::max_piece_count},
        {"metadata_token_limit", lt::settings_pack::metadata_token_limit},
    };

    return int_settings;
}

std::map<std::string, int> SettingsPack::AllowedStringSettings()
{
    static std::map<std::string, int> str_settings =
    {
        {"user_agent", lt::settings_pack::user_agent},
        {"announce_ip", lt::settings_pack::announce_ip},
        {"handshake_client_version", lt::settings_pack::handshake_client_version},
    };

    return str_settings;
}
