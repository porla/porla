#include "../types.hpp"

#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>

using porla::Lua::Types::LtSettingsPack;

void LtSettingsPack::Register(sol::state& lua)
{
    lua.new_usertype<lt::settings_pack>(
        "lt.settings_pack",
        sol::no_constructor,
        sol::meta_function::index, [](sol::this_state s, lt::settings_pack& pack, const std::string& key) -> sol::object {
            sol::state_view lua{s};

            int setting = lt::setting_by_name(key);

            if (setting == -1)
            {
                return sol::nil;
            }

            int type = setting & lt::settings_pack::type_mask;

            switch (type)
            {
                case lt::settings_pack::string_type_base:
                    return sol::make_object(lua, pack.get_str(setting));
                case lt::settings_pack::int_type_base:
                    return sol::make_object(lua, pack.get_int(setting));
                case lt::settings_pack::bool_type_base:
                    return sol::make_object(lua, pack.get_bool(setting));
            }

            return sol::nil;
        },
        sol::meta_function::new_index, [](sol::this_state s, lt::settings_pack& pack, const std::string& key, sol::object value) {
            int setting = lt::setting_by_name(key);

            if (setting == -1)
            {
                return;
            }

            int type = setting & lt::settings_pack::type_mask;

            switch (type)
            {
                case lt::settings_pack::string_type_base:
                    pack.set_str(setting, value.as<std::string>());
                    break;
                case lt::settings_pack::int_type_base:
                    pack.set_int(setting, value.as<int>());
                    break;
                case lt::settings_pack::bool_type_base:
                    pack.set_bool(setting, value.as<bool>());
                    break;
            }
        });
}
