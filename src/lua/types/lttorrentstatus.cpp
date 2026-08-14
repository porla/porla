#include "../types.hpp"

#include <libtorrent/torrent_status.hpp>

using porla::Lua::Types::LtTorrentStatus;

void LtTorrentStatus::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["torrent_status"] = lua.new_usertype<lt::torrent_status>(
        "lt.torrent_status",
        sol::no_constructor,
        "active_duration",        sol::property([](const lt::torrent_status& ts) { return ts.active_duration.count(); }),
        "added_time",             sol::readonly(&lt::torrent_status::added_time),
        "all_time_download",      sol::readonly(&lt::torrent_status::all_time_download),
        "all_time_upload",        sol::readonly(&lt::torrent_status::all_time_upload),
        "announcing_to_dht",      sol::readonly(&lt::torrent_status::announcing_to_dht),
        "announcing_to_lsd",      sol::readonly(&lt::torrent_status::announcing_to_lsd),
        "announcing_to_trackers", sol::readonly(&lt::torrent_status::announcing_to_trackers),
        "block_size",             sol::readonly(&lt::torrent_status::block_size),
        "completed_time",         sol::readonly(&lt::torrent_status::completed_time),
        "connect_candidates",     sol::readonly(&lt::torrent_status::connect_candidates),
        "connections_limit",      sol::readonly(&lt::torrent_status::connections_limit),
        "current_tracker",        sol::readonly(&lt::torrent_status::current_tracker),
        "distributed_copies",     sol::readonly(&lt::torrent_status::distributed_copies),
        "down_bandwidth_queue",   sol::readonly(&lt::torrent_status::down_bandwidth_queue),
        "download_payload_rate",  sol::readonly(&lt::torrent_status::download_payload_rate),
        "download_rate",          sol::readonly(&lt::torrent_status::download_rate),
        // errc
        // error_file
        "finished_duration",      sol::property([](const lt::torrent_status& ts) { return ts.finished_duration.count(); }),
        "flags",                  sol::readonly(&lt::torrent_status::flags),
        "info_hash",              sol::readonly(&lt::torrent_status::info_hashes),
        "is_finished",            sol::readonly(&lt::torrent_status::is_finished),
        "is_seeding",             sol::readonly(&lt::torrent_status::is_seeding),
        "handle",                 sol::readonly(&lt::torrent_status::handle),
        "has_incoming",           sol::readonly(&lt::torrent_status::has_incoming),
        "has_metadata",           sol::readonly(&lt::torrent_status::has_metadata),
        // last download
        // last seen complete
        // last upload
        "list_peers",             sol::readonly(&lt::torrent_status::list_peers),
        "list_seeds",             sol::readonly(&lt::torrent_status::list_seeds),
        "moving_storage",         sol::readonly(&lt::torrent_status::moving_storage),
        "name",                   sol::readonly(&lt::torrent_status::name),
        // "need_save_resume",       sol::readonly(&lt::torrent_status::need_save_resume),
        // next announce
        "num_complete",           sol::readonly(&lt::torrent_status::num_complete),
        "num_connections",        sol::readonly(&lt::torrent_status::num_connections),
        "num_incomplete",         sol::readonly(&lt::torrent_status::num_incomplete),
        "num_peers",              sol::readonly(&lt::torrent_status::num_peers),
        "num_pieces",             sol::readonly(&lt::torrent_status::num_pieces),
        "num_seeds",              sol::readonly(&lt::torrent_status::num_seeds),
        "num_uploads",            sol::readonly(&lt::torrent_status::num_uploads),
        // pieces
        "progress",               sol::readonly(&lt::torrent_status::progress),
        "queue_position",         sol::readonly(&lt::torrent_status::queue_position),
        "save_path",              sol::readonly(&lt::torrent_status::save_path),
        "seed_rank",              sol::readonly(&lt::torrent_status::seed_rank),
        "seeding_duration",       sol::property([](const lt::torrent_status& ts) { return ts.seeding_duration.count(); }),
        "state",                  sol::property([](const lt::torrent_status& ts) { return ts.state; }),
        // storage mode
        "torrent_file",           sol::property([](const lt::torrent_status& ts) -> std::shared_ptr<const lt::torrent_info>
                                  {
                                      if (auto tf = ts.torrent_file.lock()) return tf;
                                      return nullptr;
                                  }),
        "total",                  sol::readonly(&lt::torrent_status::total),
        "total_done",             sol::readonly(&lt::torrent_status::total_done),
        "total_download",         sol::readonly(&lt::torrent_status::total_download),
        "total_failed_bytes",     sol::readonly(&lt::torrent_status::total_failed_bytes),
        "total_payload_download", sol::readonly(&lt::torrent_status::total_payload_download),
        "total_payload_upload",   sol::readonly(&lt::torrent_status::total_payload_upload),
        "total_redundant_bytes",  sol::readonly(&lt::torrent_status::total_redundant_bytes),
        "total_upload",           sol::readonly(&lt::torrent_status::total_upload),
        "up_bandwidth_queue",     sol::readonly(&lt::torrent_status::up_bandwidth_queue),
        "upload_payload_rate",    sol::readonly(&lt::torrent_status::upload_payload_rate),
        "upload_rate",            sol::readonly(&lt::torrent_status::upload_rate),
        "uploads_limit",          sol::readonly(&lt::torrent_status::uploads_limit)
        // verified pieces
        );

    auto state_type = lua.new_usertype<lt::torrent_status::state_t>(
        "lt.torrent_status.state_t",
        sol::no_constructor,
        sol::meta_function::equal_to, [](lt::torrent_status::state_t a, lt::torrent_status::state_t b) { return a == b; }
    );

    state_type["checking_files"]       = sol::var(lt::torrent_status::state_t::checking_files);
    state_type["downloading_metadata"] = sol::var(lt::torrent_status::state_t::downloading_metadata);
    state_type["downloading"]          = sol::var(lt::torrent_status::state_t::downloading);
    state_type["finished"]             = sol::var(lt::torrent_status::state_t::finished);
    state_type["seeding"]              = sol::var(lt::torrent_status::state_t::seeding);
    state_type["checking_resume_data"] = sol::var(lt::torrent_status::state_t::checking_resume_data);

    lt["torrent_status"]["state_t"] = state_type;
}
