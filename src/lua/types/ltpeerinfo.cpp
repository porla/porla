#include "../types.hpp"

#include <libtorrent/peer_info.hpp>

using porla::Lua::Types::LtPeerInfo;

void LtPeerInfo::Register(sol::state& lua)
{
    lua.new_usertype<lt::peer_info>(
        "lt.peer_info",
        sol::no_constructor,
        "busy_requests",            sol::readonly(&lt::peer_info::busy_requests),
        "client",                   sol::readonly(&lt::peer_info::client),
        // connection_type
        "down_speed",               sol::readonly(&lt::peer_info::down_speed),
        "download_queue_length",    sol::readonly(&lt::peer_info::download_queue_length),
        "download_rate_peak",       sol::readonly(&lt::peer_info::download_rate_peak),
        // download_queue_time
        "downloading_block_index",  sol::readonly(&lt::peer_info::downloading_block_index),
        // downloading_piece_index
        "downloading_progress",     sol::readonly(&lt::peer_info::downloading_progress),
        "downloading_total",        sol::readonly(&lt::peer_info::downloading_total),
        "failcount",                sol::readonly(&lt::peer_info::failcount),
        // flags
        // last_active
        // last_request
        // local_endpoint
        "num_hashfails",            sol::readonly(&lt::peer_info::num_hashfails),
        "num_pieces",               sol::readonly(&lt::peer_info::num_hashfails),
        "payload_down_speed",       sol::readonly(&lt::peer_info::payload_down_speed),
        "payload_up_speed",         sol::readonly(&lt::peer_info::payload_up_speed),
        "pending_disk_bytes",       sol::readonly(&lt::peer_info::pending_disk_bytes),
        "pending_disk_read_bytes",  sol::readonly(&lt::peer_info::pending_disk_read_bytes),
        // pid
        // pieces
        "progress",                 sol::readonly(&lt::peer_info::progress),
        "queue_bytes",              sol::readonly(&lt::peer_info::queue_bytes),
        // read_state
        "receive_buffer_size",      sol::readonly(&lt::peer_info::receive_buffer_size),
        "receive_buffer_watermark", sol::readonly(&lt::peer_info::receive_buffer_watermark),
        "receive_quota",            sol::readonly(&lt::peer_info::receive_quota),
        // remote_endpoint
        "request_timeout",          sol::readonly(&lt::peer_info::request_timeout),
        "requests_in_buffer",       sol::readonly(&lt::peer_info::requests_in_buffer),
        "rtt",                      sol::readonly(&lt::peer_info::rtt),
        // source
        "send_buffer_size",         sol::readonly(&lt::peer_info::send_buffer_size),
        "send_quota",               sol::readonly(&lt::peer_info::send_quota),
        "target_dl_queue_length",   sol::readonly(&lt::peer_info::target_dl_queue_length),
        "timed_out_requests",       sol::readonly(&lt::peer_info::timed_out_requests),
        "total_download",           sol::readonly(&lt::peer_info::total_download),
        "total_upload",             sol::readonly(&lt::peer_info::total_upload),
        "up_speed",                 sol::readonly(&lt::peer_info::up_speed),
        "upload_queue_length",      sol::readonly(&lt::peer_info::upload_queue_length),
        "upload_rate_peak",         sol::readonly(&lt::peer_info::upload_rate_peak),
        "used_send_buffer",         sol::readonly(&lt::peer_info::used_send_buffer),
        "used_receive_buffer",      sol::readonly(&lt::peer_info::used_receive_buffer)
        // write_state
    );
}
