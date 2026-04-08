#include "../types.hpp"

#include <libtorrent/announce_entry.hpp>

using porla::Lua::Types::LtAnnounceEndpoint;

void LtAnnounceEndpoint::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["announce_endpoint"] = lua.new_usertype<lt::announce_endpoint>(
        "lt.announce_endpoint",
        sol::no_constructor,
        "enabled", sol::readonly(&lt::announce_endpoint::enabled),
        "info_hashes", sol::readonly(&lt::announce_endpoint::info_hashes),
        "local_endpoint", sol::property([](const lt::announce_endpoint& ae)
        {
            return std::make_tuple(
                ae.local_endpoint.address().to_string(),
                ae.local_endpoint.port());
        }));
}
