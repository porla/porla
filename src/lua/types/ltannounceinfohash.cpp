#include "../types.hpp"

#include <libtorrent/announce_entry.hpp>

using porla::Lua::Types::LtAnnounceInfohash;

void LtAnnounceInfohash::Register(sol::state& lua)
{
    lua.new_usertype<lt::announce_infohash>(
        "lt.announce_infohash",
        sol::no_constructor,
        "complete_sent",     sol::readonly_property([](const lt::announce_infohash& ai) { return ai.complete_sent; }),
        "fails",             sol::readonly_property([](const lt::announce_infohash& ai) { return ai.fails; }),
        // last_error
        "message",           &lt::announce_infohash::message,
        "min_announce",      sol::readonly_property([](const lt::announce_infohash& ai) { return ai.min_announce.time_since_epoch().count(); }),
        "next_announce",     sol::readonly_property([](const lt::announce_infohash& ai) { return ai.next_announce.time_since_epoch().count(); }),
        "scrape_complete",   &lt::announce_infohash::scrape_complete,
        "scrape_downloaded", &lt::announce_infohash::scrape_downloaded,
        "scrape_incomplete", &lt::announce_infohash::scrape_incomplete,
        "start_sent",        sol::readonly_property([](const lt::announce_infohash& ai) { return ai.start_sent; }),
        "updating",          sol::readonly_property([](const lt::announce_infohash& ai) { return ai.updating; }));

}
