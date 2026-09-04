#include "../types.hpp"

#include <libtorrent/torrent_handle.hpp>

#include "../../torrentclientdata.hpp"

using porla::Lua::Types::LtTorrentHandle;

void LtTorrentHandle::Register(sol::state& lua)
{
    lua.new_usertype<lt::torrent_handle>(
        "lt.torrent_handle",
        sol::no_constructor,
        // add_piece
        // add_tracker
        "add_url_seed",               &lt::torrent_handle::add_url_seed,
        "clear_error",                &lt::torrent_handle::clear_error,
        "clear_peers",                &lt::torrent_handle::clear_peers,
        "clear_piece_deadlines",      &lt::torrent_handle::clear_piece_deadlines,
        // connect_peer
        "download_limit",             &lt::torrent_handle::download_limit,
        // file_priority
        // file_progress
        "file_status",                &lt::torrent_handle::file_status,
        // flags
        "flush_cache",                &lt::torrent_handle::flush_cache,
        "force_dht_announce",         &lt::torrent_handle::force_dht_announce,
        "force_lsd_announce",         &lt::torrent_handle::force_lsd_announce,
        // force_reannounce
        "force_recheck",              &lt::torrent_handle::force_recheck,
        //"get_download_queue",       &lt::torrent_handle::get_download_queue,
        // get_file_priorities
        // get_peer_info
        "get_peer_info",              [](const lt::torrent_handle& th)
                                        {
                                            std::vector<lt::peer_info> peers;
                                            th.get_peer_info(peers);
                                            return peers;
                                        },
        // get_piece_priorities
        "get_renamed_files",          &lt::torrent_handle::get_renamed_files,
        // get_resume_data
        "have_piece",                 &lt::torrent_handle::have_piece,
        "in_session",                 &lt::torrent_handle::in_session,
        "info_hash",                  &lt::torrent_handle::info_hashes,
        "is_valid",                   &lt::torrent_handle::is_valid,
        "max_connections",            &lt::torrent_handle::max_connections,
        "max_uploads",                &lt::torrent_handle::max_uploads,
        "move_storage",               &lt::torrent_handle::move_storage,
        // "need_save_resume_data"
        "pause",                      &lt::torrent_handle::pause,
        // piece_availability
        //"piece_layers",               &lt::torrent_handle::piece_layers,
        // piece_priority
        "post_download_queue",        &lt::torrent_handle::post_download_queue,
        "post_file_priorities",       &lt::torrent_handle::post_file_priorities,
        "post_file_progress",         &lt::torrent_handle::post_file_progress,
        "post_file_status",           &lt::torrent_handle::post_file_status,
        "post_peer_info",             &lt::torrent_handle::post_peer_info,
        "post_piece_availability",    &lt::torrent_handle::post_piece_availability,
        "post_status",                &lt::torrent_handle::post_status,
        "post_trackers",              &lt::torrent_handle::post_trackers,
        // prioritize_files
        // prioritize_pieces
        "queue_position",             &lt::torrent_handle::queue_position,
        "queue_position_bottom",      &lt::torrent_handle::queue_position_bottom,
        "queue_position_down",        &lt::torrent_handle::queue_position_down,
        "queue_position_set",         &lt::torrent_handle::queue_position_set,
        "queue_position_top",         &lt::torrent_handle::queue_position_top,
        "queue_position_up",          &lt::torrent_handle::queue_position_up,
        "read_piece",                 [](const lt::torrent_handle& th, int piece_index)
                                        { th.read_piece(lt::piece_index_t{piece_index}); },
        "remove_url_seed",            &lt::torrent_handle::remove_url_seed,
        "rename_file",                &lt::torrent_handle::rename_file,
        // replace_trackers
        // reset_piece_deadline
        "resume",                     &lt::torrent_handle::resume,
        // save_resume_data
        // scrape_tracker
        "set_download_limit",         &lt::torrent_handle::set_download_limit,
        // set_flags
        "set_max_connections",        &lt::torrent_handle::set_max_connections,
        "set_max_uploads",            &lt::torrent_handle::set_max_uploads,
        // set_metadata
        // set_piece_deadline
        // set_sequential_range
        "set_ssl_certificate",        &lt::torrent_handle::set_ssl_certificate,
        "set_ssl_certificate_buffer", &lt::torrent_handle::set_ssl_certificate_buffer,
        "set_upload_limit",           &lt::torrent_handle::set_upload_limit,
        "status",                     [](const lt::torrent_handle& th) { return th.status(); },
        "torrent_file",               &lt::torrent_handle::torrent_file,
        "trackers",                   &lt::torrent_handle::trackers,
        // unset_flags
        "upload_limit",               &lt::torrent_handle::upload_limit,
        "url_seeds",                  &lt::torrent_handle::url_seeds);
}
