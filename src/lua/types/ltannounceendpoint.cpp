#include "../types.hpp"

#include <libtorrent/announce_entry.hpp>

using porla::Lua::Types::LtAnnounceEndpoint;

void LtAnnounceEndpoint::Register(sol::state& lua)
{
    lua.new_usertype<lt::announce_endpoint>(
        "LtAnnounceEndpoint",
        sol::no_constructor,
        "enabled", sol::readonly(&lt::announce_endpoint::enabled),
        "info_hashes", sol::property([](const lt::announce_endpoint& ep) { return sol::as_table(ep.info_hashes); }),
        "local_endpoint", sol::property([](const lt::announce_endpoint& ae)
        {
            return std::make_tuple(
                ae.local_endpoint.address().to_string(),
                ae.local_endpoint.port());
        }));
}
