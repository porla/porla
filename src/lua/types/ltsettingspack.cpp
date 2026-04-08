#include "../types.hpp"

#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>

using porla::Lua::Types::LtSettingsPack;

void LtSettingsPack::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["settings_pack"] = lua.new_usertype<lt::settings_pack>(
        "lt.settings_pack",
        sol::constructors<lt::settings_pack()>(),

        "clear", sol::overload(
            [](lt::settings_pack& sp)           { sp.clear(); },
            [](lt::settings_pack& sp, int name) { sp.clear(name); }
        ),

        "has_val", &lt::settings_pack::has_val,

        // get & set
        "get_bool", [](const lt::settings_pack& sp, int name) { return sp.get_bool(name); },
        "get_int",  [](const lt::settings_pack& sp, int name) { return sp.get_int(name); },
        "get_str",  [](const lt::settings_pack& sp, int name) { return sp.get_str(name); },

        "set_bool", [](lt::settings_pack& sp, int name, bool val)        { sp.set_bool(name, val); },
        "set_int",  [](lt::settings_pack& sp, int name, int val)         { sp.set_int(name, val); },
        "set_str",  [](lt::settings_pack& sp, int name, std::string val) { sp.set_str(name, val); });

    // the lt settings packs
    lt["default_settings"]      = sol::factories([]() { return lt::default_settings(); });
    lt["high_performance_seed"] = sol::factories([]() { return lt::high_performance_seed(); });
    lt["min_memory_usage"]      = sol::factories([]() { return lt::min_memory_usage(); });

    lt["setting_by_name"] = [](const std::string& name) -> sol::optional<int>
    {
        int val = lt::setting_by_name(name);
        if (val == -1) return sol::nullopt;
        return val;
    };
}
