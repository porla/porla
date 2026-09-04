#include "../types.hpp"

#include <libtorrent/announce_entry.hpp>

using porla::Lua::Types::LtAnnounceEntry;

void LtAnnounceEntry::Register(sol::state& lua)
{
    lua.new_usertype<lt::announce_entry>(
        "lt.announce_entry",
        sol::no_constructor,
        "endpoints",  sol::readonly(&lt::announce_entry::endpoints),
        "fail_limit", sol::readonly(&lt::announce_entry::fail_limit),
        "source",     sol::property([](const lt::announce_entry& ae) { return ae.source; }),
        "tier",       sol::readonly(&lt::announce_entry::tier),
        "trackerid",  sol::readonly(&lt::announce_entry::trackerid),
        "url",        sol::readonly(&lt::announce_entry::url),
        "verified",   sol::property([](const lt::announce_entry& ae) { return ae.verified; }));
}
