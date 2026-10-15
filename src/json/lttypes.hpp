#pragma once

#include <libtorrent/alert_types.hpp>
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/download_priority.hpp>
#include <libtorrent/torrent_flags.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/units.hpp>
#include <nlohmann/json.hpp>

namespace libtorrent
{
    inline void from_json(const nlohmann::json& json, announce_endpoint& endpoint) {}
    inline void from_json(const nlohmann::json& json, announce_entry& entry) {}
    inline void from_json(const nlohmann::json& json, announce_infohash& infohash) {}
    inline void from_json(const nlohmann::json& json, download_priority_t& dlp) { dlp = download_priority_t{json.get<uint8_t>()}; }
    void from_json(const nlohmann::json& json, info_hash_t& infohash);
    inline void from_json(const nlohmann::json& json, peer_info& pi) {}
    void from_json(const nlohmann::json& json, queue_position_t& pos);
    void from_json(const nlohmann::json& json, settings_pack& settings);
    void from_json(const nlohmann::json& json, torrent_flags_t& flags);
    inline void from_json(const nlohmann::json& json, torrent_status& ts) {}
    void from_json(const nlohmann::json& j, typed_bitfield<piece_index_t>& pieces);

    void to_json(nlohmann::json& json, const announce_endpoint& endpoint);
    void to_json(nlohmann::json& json, const announce_entry& entry);
    void to_json(nlohmann::json& json, const announce_infohash& infohash);
    inline void to_json(nlohmann::json& json, const download_priority_t& p) { json = static_cast<uint8_t>(p); }
    void to_json(nlohmann::json& json, const torrent_flags_t& flags);
    void to_json(nlohmann::json& json, const info_hash_t& infohash);
    void to_json(nlohmann::json& json, const peer_info& pi);
    void to_json(nlohmann::json& json, const queue_position_t& pos);
    void to_json(nlohmann::json& json, const settings_pack& settings);
    void to_json(nlohmann::json& json, const torrent_status& ts);
    void to_json(nlohmann::json& json, const lt::torrent_status::state_t& state);
    void to_json(nlohmann::json& j, const typed_bitfield<piece_index_t>& pieces);
    void to_json(nlohmann::json& j, const libtorrent::file_flags_t& flags);

    // alerts
    void to_json(nlohmann::json& json, const add_torrent_alert& alert);
    void to_json(nlohmann::json& json, const dht_stats_alert& alert);
    void to_json(nlohmann::json& json, const file_error_alert& alert);
    void to_json(nlohmann::json& json, const metadata_received_alert& alert);
    void to_json(nlohmann::json& json, const state_changed_alert& alert);
    void to_json(nlohmann::json& json, const storage_moved_alert& alert);
    void to_json(nlohmann::json& json, const storage_moved_failed_alert& alert);
    void to_json(nlohmann::json& json, const torrent_checked_alert& alert);
    void to_json(nlohmann::json& json, const torrent_error_alert& alert);
    void to_json(nlohmann::json& json, const torrent_finished_alert& alert);
    void to_json(nlohmann::json& json, const torrent_need_cert_alert& alert);
    void to_json(nlohmann::json& json, const torrent_paused_alert& alert);
    void to_json(nlohmann::json& json, const torrent_resumed_alert& alert);
    void to_json(nlohmann::json& json, const tracker_error_alert& alert);
    void to_json(nlohmann::json& json, const tracker_reply_alert& alert);
    void to_json(nlohmann::json& json, const tracker_warning_alert& alert);

    namespace aux
    {
        inline void to_json(nlohmann::json& j, const file_index_t& fi)   { j = static_cast<int>(fi); }
        inline void from_json(const nlohmann::json& j, file_index_t& fi) { fi = file_index_t{ j.get<int>() }; }
    }
}
