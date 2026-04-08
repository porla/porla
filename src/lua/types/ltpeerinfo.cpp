#include "../types.hpp"

#include <libtorrent/peer_info.hpp>

using porla::Lua::Types::LtPeerInfo;

void LtPeerInfo::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["peer_info"] = lua.new_usertype<lt::peer_info>(
        "lt.peer_info",
        sol::no_constructor,
        "busy_requests",         sol::readonly(&lt::peer_info::busy_requests),
        "client",                sol::readonly(&lt::peer_info::client),
        "connection_type",       sol::readonly(&lt::peer_info::connection_type),
        "down_speed",            sol::readonly(&lt::peer_info::down_speed),
        "download_queue_length", sol::readonly(&lt::peer_info::download_queue_length),
        "download_queue_time",   sol::readonly(&lt::peer_info::download_queue_time),
        "flags",                 sol::readonly(&lt::peer_info::flags),
        "last_active",           sol::property([](const lt::peer_info& pi) { return pi.last_active.count(); }),
        "last_request",          sol::property([](const lt::peer_info& pi) { return pi.last_request.count(); }),
        "progress",              sol::readonly(&lt::peer_info::progress),
        "rtt",                   sol::readonly(&lt::peer_info::rtt),
        "source",                sol::readonly(&lt::peer_info::source),
        "total_download",        sol::readonly(&lt::peer_info::total_download),
        "total_upload",          sol::readonly(&lt::peer_info::total_upload),
        "up_speed",              sol::readonly(&lt::peer_info::up_speed));
}
