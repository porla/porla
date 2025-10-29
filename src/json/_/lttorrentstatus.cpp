#include "../all.hpp"

#include <libtorrent/torrent_status.hpp>

#include "../../torrentclientdata.hpp"

namespace libtorrent
{
    void from_json(const nlohmann::json& j, typed_bitfield<piece_index_t>& pieces)
    {
    }

    void to_json(nlohmann::json& j, const typed_bitfield<piece_index_t>& pieces)
    {
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
            {"distributed_full_copies", ts.distributed_full_copies},
            {"down_bandwidth_queue", ts.down_bandwidth_queue},
            {"download_rate", ts.download_rate},
            {"download_payload_rate", ts.download_payload_rate},
            {"errc", ts.errc},
            {"finished_duration", ts.finished_duration.count()},
            {"flags", static_cast<std::uint64_t>(ts.flags)},
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
            {"need_save_resume", ts.need_save_resume},
            {"next_announce", ts.next_announce.count()},
            {"num_complete", ts.num_complete},
            {"num_connections", ts.num_connections},
            {"num_incomplete", ts.num_incomplete},
            {"num_peers", ts.num_peers},
            {"num_pieces", ts.num_pieces},
            {"num_seeds", ts.num_seeds},
            {"num_uploads", ts.num_uploads},
            {"pieces", ts.pieces},
            {"progress", ts.progress},
            {"queue_position", static_cast<int>(ts.queue_position)},
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
            {"uploads_limit", ts.uploads_limit},
            {"verified_pieces", ts.verified_pieces}
        };
    }
}
