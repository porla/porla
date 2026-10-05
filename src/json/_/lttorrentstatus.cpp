#include "../all.hpp"

#include <libtorrent/torrent_status.hpp>

#include "../../torrentclientdata.hpp"
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

        j = nlohmann::json::object();
        j["$userdata"] = userdata_json;
        j["active_duration"] = ts.active_duration.count();
        j["added_time"] = ts.added_time;
        j["all_time_download"] = ts.all_time_download;
        j["all_time_upload"] = ts.all_time_upload;
        j["announcing_to_dht"] = ts.announcing_to_dht;
        j["announcing_to_lsd"] = ts.announcing_to_lsd;
        j["announcing_to_trackers"] = ts.announcing_to_trackers;
        j["block_size"] = ts.block_size;
        j["completed_time"] = ts.completed_time;
        j["connect_candidates"] = ts.connect_candidates;
        j["connections_limit"] = ts.connections_limit;
        j["current_tracker"] = ts.current_tracker;
        j["distributed_copies"] = ts.distributed_copies;
        j["down_bandwidth_queue"] = ts.down_bandwidth_queue;
        j["download_rate"] = ts.download_rate;
        j["download_payload_rate"] = ts.download_payload_rate;
        j["errc"] = ts.errc;
        j["finished_duration"] = ts.finished_duration.count();
        j["flags"] = ts.flags;
        j["has_incoming"] = ts.has_incoming;
        j["has_metadata"] = ts.has_metadata;
        j["info_hash"] = ts.info_hashes;
        j["is_finished"] = ts.is_finished;
        j["is_seeding"] = ts.is_seeding;
        j["last_download"] = ts.last_download.time_since_epoch().count() != 0
            ? lt::total_seconds(lt::clock_type::now() - ts.last_download)
            : -1;
        j["last_seen_complete"] = ts.last_seen_complete;
        j["last_upload"] = ts.last_upload.time_since_epoch().count() != 0
            ? lt::total_seconds(lt::clock_type::now() - ts.last_upload)
            : -1;
        j["list_peers"] = ts.list_peers;
        j["list_seeds"] = ts.list_seeds;
        j["moving_storage"] = ts.moving_storage;
        j["name"] = ts.name;
        j["need_save_resume_data"] = ts.need_save_resume_data;
        j["next_announce"] = lt::total_seconds(ts.next_announce);
        j["num_complete"] = ts.num_complete;
        j["num_connections"] = ts.num_connections;
        j["num_incomplete"] = ts.num_incomplete;
        j["num_peers"] = ts.num_peers;
        j["num_pieces"] = ts.num_pieces;
        j["num_seeds"] = ts.num_seeds;
        j["num_uploads"] = ts.num_uploads;
        j["progress"] = ts.progress;
        j["queue_position"] = static_cast<int>(ts.queue_position);
        j["ratio"] = porla::Utils::Ratio(ts);
        j["ratio_real"] = porla::Utils::RealRatio(ts);
        j["save_path"] = ts.save_path;
        j["seed_rank"] = ts.seed_rank;
        j["seeding_duration"] = ts.seeding_duration.count();
        j["state"] = ts.state;
        j["storage_mode"] = ts.storage_mode;
        j["total"] = ts.total;
        j["total_done"] = ts.total_done;
        j["total_download"] = ts.total_download;
        j["total_failed_bytes"] = ts.total_failed_bytes;
        j["total_payload_download"] = ts.total_payload_download;
        j["total_payload_upload"] = ts.total_payload_upload;
        j["total_redundant_bytes"] = ts.total_redundant_bytes;
        j["total_upload"] = ts.total_upload;
        j["total_wanted"] = ts.total_wanted;
        j["total_wanted_done"] = ts.total_wanted_done;
        j["up_bandwidth_queue"] = ts.up_bandwidth_queue;
        j["upload_payload_rate"] = ts.upload_payload_rate;
        j["upload_rate"] = ts.upload_rate;
        j["uploads_limit"] = ts.uploads_limit;
    }
}
