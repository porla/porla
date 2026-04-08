#include "../types.hpp"

#include <libtorrent/torrent_handle.hpp>

#include "../../torrentclientdata.hpp"

using porla::Lua::Types::LtTorrentHandle;

void LtTorrentHandle::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["torrent_handle"] = lua.new_usertype<lt::torrent_handle>(
        "lt.torrent_handle",
        sol::no_constructor,
        "clear_error",             &lt::torrent_handle::clear_error,
        "clear_peers",             &lt::torrent_handle::clear_peers,
        "download_limit",          &lt::torrent_handle::download_limit,
        "file_priorities",         &lt::torrent_handle::get_file_priorities,
        "flags",                   &lt::torrent_handle::flags,
        "flush_cache",             &lt::torrent_handle::flush_cache,
        "force_reannounce",        [](const lt::torrent_handle& th, const sol::table& args)
                                   {
                                       int seconds = 0;
                                       int index   = -1;

                                       if (args["seconds"].is<int>())       seconds = args["seconds"];
                                       if (args["tracker_index"].is<int>()) index   = args["tracker_index"];

                                       th.force_reannounce(seconds, index);
                                   },
        "force_recheck",           &lt::torrent_handle::force_recheck,
        "info_hash",               &lt::torrent_handle::info_hashes,
        "is_valid",                &lt::torrent_handle::is_valid,
        "max_connections",         &lt::torrent_handle::max_connections,
        "max_uploads",             &lt::torrent_handle::max_uploads,
        "move_storage",            [](const lt::torrent_handle& th, const std::string& path) { th.move_storage(path); },
        "pause",                   [](const lt::torrent_handle& th) { return th.pause(); },
        "peer_info",               [](const lt::torrent_handle& th)
                                   {
                                       std::vector<lt::peer_info> peers;
                                       th.get_peer_info(peers);
                                       return peers;
                                   },
        "post_download_queue",     &lt::torrent_handle::post_download_queue,
        "post_file_progress",      [](const lt::torrent_handle& th) { return th.post_file_progress(lt::torrent_handle::piece_granularity); },
        "post_peer_info",          &lt::torrent_handle::post_peer_info,
        "post_piece_availability", &lt::torrent_handle::post_piece_availability,
        "post_status",             [](const lt::torrent_handle& th) { return th.post_status(); },
        "post_trackers",           &lt::torrent_handle::post_trackers,
        "prioritize_files",        [](const lt::torrent_handle& th, const sol::table& args)
        {
            std::vector<int> priorities = args.as<std::vector<int>>();
            std::vector<lt::download_priority_t> prios;
            std::for_each(
                priorities.begin(),
                priorities.end(),
                [&prios](int i) { prios.emplace_back(i); });
            th.prioritize_files(prios);
        },
        "queue_position",          &lt::torrent_handle::queue_position,
        "queue_position_bottom",   &lt::torrent_handle::queue_position_bottom,
        "queue_position_down",     &lt::torrent_handle::queue_position_down,
        "queue_position_top",      &lt::torrent_handle::queue_position_top,
        "queue_position_up",       &lt::torrent_handle::queue_position_up,
        "resume",                  &lt::torrent_handle::resume,
        "set_download_limit",      &lt::torrent_handle::set_download_limit,
        "set_flags",               sol::overload(
            [](const lt::torrent_handle& th, const lt::torrent_flags_t& flags) { th.set_flags(flags); },
            [](const lt::torrent_handle& th, const lt::torrent_flags_t& flags, const lt::torrent_flags_t& mask) { th.set_flags(flags, mask); }
        ),
        "set_max_connections",     &lt::torrent_handle::set_max_connections,
        "set_max_uploads",         &lt::torrent_handle::set_max_uploads,
        "set_upload_limit",        &lt::torrent_handle::set_upload_limit,
        "status",                  [](const lt::torrent_handle& th) { return th.status(); },
        "trackers",                &lt::torrent_handle::trackers,
        "torrent_file",            &lt::torrent_handle::torrent_file,
        "upload_limit",            &lt::torrent_handle::upload_limit,
        "userdata",                [](const lt::torrent_handle& th) { return th.userdata().get<TorrentClientData>(); });
}
