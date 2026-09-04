#include "../types.hpp"

#include <libtorrent/download_priority.hpp>

using porla::Lua::Types::LtDownloadPriority;

void LtDownloadPriority::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["download_priority_t"] = lua.new_usertype<lt::download_priority_t>(
        "lt.download_priority_t",
        sol::no_constructor,
        sol::meta_function::equal_to, [](lt::download_priority_t a, lt::download_priority_t b) { return a == b; }
    );

    lt["dont_download"]    = sol::var(lt::dont_download);
    lt["default_priority"] = sol::var(lt::default_priority);
    lt["low_priority"]     = sol::var(lt::low_priority);
    lt["top_priority"]     = sol::var(lt::top_priority);
}
