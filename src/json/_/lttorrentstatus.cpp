#include "../all.hpp"

#include <libtorrent/torrent_status.hpp>

#include "../../torrentclientdata.hpp"
#include "../../utils/base64.hpp"
#include "../../utils/ratio.hpp"

namespace libtorrent
{
    void to_json(nlohmann::json& j, const lt::torrent_status::state_t& state)
    {
        if (state == lt::torrent_status::state_t::checking_files)       j = "checking_files";
        if (state == lt::torrent_status::state_t::downloading_metadata) j = "downloading_metadata";
        if (state == lt::torrent_status::state_t::downloading)          j = "downloading";
        if (state == lt::torrent_status::state_t::finished)             j = "finished";
        if (state == lt::torrent_status::state_t::seeding)              j = "seeding";
        if (state == lt::torrent_status::state_t::checking_resume_data) j = "checking_resume_data";
    }

    void to_json(nlohmann::json& j, const lt::resume_data_flags_t& resume_data_flags)
    {
        std::unordered_set<std::string> flags;

        if (resume_data_flags & lt::torrent_handle::flush_disk_cache)     flags.insert("flush_disk_cache");
        if (resume_data_flags & lt::torrent_handle::save_info_dict)       flags.insert("save_info_dict");
        if (resume_data_flags & lt::torrent_handle::if_counters_changed)  flags.insert("if_counters_changed");
        if (resume_data_flags & lt::torrent_handle::if_download_progress) flags.insert("if_download_progress");
        if (resume_data_flags & lt::torrent_handle::if_config_changed)    flags.insert("if_config_changed");
        if (resume_data_flags & lt::torrent_handle::if_state_changed)     flags.insert("if_state_changed");
        if (resume_data_flags & lt::torrent_handle::if_metadata_changed)  flags.insert("if_metadata_changed");

        j = flags;
    }

    void to_json(nlohmann::json& j, const lt::storage_mode_t& storage_mode)
    {
        if (storage_mode == lt::storage_mode_t::storage_mode_allocate) j = "allocate";
        if (storage_mode == lt::storage_mode_t::storage_mode_sparse)   j = "sparse";
    }

    void to_json(nlohmann::json &j, const torrent_status& ts)
    {
        const auto userdata = ts.handle.is_valid()
            ? ts.handle.userdata().get<porla::TorrentClientData>()
            : nullptr;

        nlohmann::json userdata_json;

        if (userdata != nullptr)
        {
            userdata_json = *userdata;
        }

        j = {
            {"$userdata", userdata_json},
            {"active_duration", ts.active_duration.count()},
            {"added_time", ts.added_time},
            {"all_time_download", ts.all_time_download},
            {"all_time_upload", ts.all_time_upload},
            {"announcing_to_dht", ts.announcing_to_dht},
            {"announcing_to_lsd", ts.announcing_to_lsd},
            {"announcing_to_trackers", ts.announcing_to_trackers},
            {"block_size", ts.block_size},
            {"completed_time", ts.completed_time},
            {"connect_candidates", ts.connect_candidates},
            {"connections_limit", ts.connections_limit},
            {"current_tracker", ts.current_tracker},
            {"distributed_copies", ts.distributed_copies},
            {"down_bandwidth_queue", ts.down_bandwidth_queue},
            {"download_rate", ts.download_rate},
            {"download_payload_rate", ts.download_payload_rate},
            {"errc", ts.errc},
            {"finished_duration", ts.finished_duration.count()},
            {"flags", ts.flags},
            {"has_incoming", ts.has_incoming},
            {"has_metadata", ts.has_metadata},
            {"info_hash", ts.info_hashes},
            {"is_finished", ts.is_finished},
            {"is_seeding", ts.is_seeding},
            {"last_download", ts.last_download.time_since_epoch().count() > 0
                ? lt::total_seconds(lt::clock_type::now() - ts.last_download)
                : -1},
            {"last_seen_complete", ts.last_seen_complete},
            {"last_upload", ts.last_upload.time_since_epoch().count() > 0
                ? lt::total_seconds(lt::clock_type::now() - ts.last_upload)
                : -1},
            {"list_peers", ts.list_peers},
            {"list_seeds", ts.list_seeds},
            {"moving_storage", ts.moving_storage},
            {"name", ts.name},
            {"need_save_resume_data", ts.need_save_resume_data},
            {"next_announce", lt::total_seconds(ts.next_announce)},
            {"num_complete", ts.num_complete},
            {"num_connections", ts.num_connections},
            {"num_incomplete", ts.num_incomplete},
            {"num_peers", ts.num_peers},
            {"num_pieces", ts.num_pieces},
            {"num_seeds", ts.num_seeds},
            {"num_uploads", ts.num_uploads},
            {"progress", ts.progress},
            {"queue_position", static_cast<int>(ts.queue_position)},
            {"ratio", porla::Utils::Ratio(ts, false)},
            {"ratio_real", porla::Utils::Ratio(ts, true)},
            {"save_path", ts.save_path},
            {"seed_rank", ts.seed_rank},
            {"seeding_duration", ts.seeding_duration.count()},
            {"state", ts.state},
            {"storage_mode", ts.storage_mode},
            {"total", ts.total},
            {"total_done", ts.total_done},
            {"total_download", ts.total_download},
            {"total_failed_bytes", ts.total_failed_bytes},
            {"total_payload_download", ts.total_payload_download},
            {"total_payload_upload", ts.total_payload_upload},
            {"total_redundant_bytes", ts.total_redundant_bytes},
            {"total_upload", ts.total_upload},
            {"total_wanted", ts.total_wanted},
            {"total_wanted_done", ts.total_wanted_done},
            {"up_bandwidth_queue", ts.up_bandwidth_queue},
            {"upload_payload_rate", ts.upload_payload_rate},
            {"upload_rate", ts.upload_rate},
            {"uploads_limit", ts.uploads_limit}
        };
    }
}
