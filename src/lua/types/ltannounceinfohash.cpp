#include "../types.hpp"

#include <libtorrent/announce_entry.hpp>

using porla::Lua::Types::LtAnnounceInfohash;

void LtAnnounceInfohash::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["announce_infohash"] = lua.new_usertype<lt::announce_infohash>(
        "lt.announce_infohash",
        sol::no_constructor,
        "complete_sent", sol::property([](const lt::announce_infohash& aih) { return aih.complete_sent; }),
        "fails", sol::property([](const lt::announce_infohash& aih) { return aih.fails; }),
        "last_error", sol::readonly(&lt::announce_infohash::last_error),
        "message", sol::readonly(&lt::announce_infohash::message),
        "min_announce", sol::property([](const lt::announce_infohash& aih) { return aih.min_announce.time_since_epoch().count(); }),
        "next_announce", sol::property([](const lt::announce_infohash& aih) { return aih.next_announce.time_since_epoch().count(); }),
        "scrape_complete", sol::readonly(&lt::announce_infohash::scrape_complete),
        "scrape_downloaded", sol::readonly(&lt::announce_infohash::scrape_downloaded),
        "scrape_incomplete", sol::readonly(&lt::announce_infohash::scrape_incomplete),
        "start_sent", sol::property([](const lt::announce_infohash& aih) { return aih.start_sent; }),
        "updating", sol::property([](const lt::announce_infohash& aih) { return aih.updating; })
    );
}
