#include "lttorrenthandle.hpp"

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

using porla::Lua::UserTypes::LibtorrentTorrentHandle;

void LibtorrentTorrentHandle::Register(sol::state& lua)
{
    sol::usertype<lt::torrent_handle> type = lua.new_usertype<lt::torrent_handle>(
        "lt_torrent_handle",
        sol::no_constructor);

    type["is_valid"] = &lt::torrent_handle::is_valid;
    type["status"] = [](const lt::torrent_handle& self) { return self.status(); };

    sol::usertype<lt::torrent_status> status = lua.new_usertype<lt::torrent_status>(
        "lt_torrent_status",
        sol::no_constructor);

    status["name"] = sol::readonly_property(&lt::torrent_status::name);
}
