#include "sessions.hpp"

#include <boost/log/trivial.hpp>

#include "../plugins/plugin.hpp"
#include "../../sessions.hpp"

using porla::Lua::Packages::Sessions;
using porla::Lua::Plugins::PluginLoadOptions;

void Sessions::Register(sol::state& lua)
{
    lua["package"]["preload"]["sessions"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table sessions = lua.create_table();

        sessions["settings"] = lua.create_table();
        sessions["settings"]["get"] = [](sol::this_state s, const std::string& session) -> sol::reference
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();
            const auto state = options.sessions.Get(session);

            if (state == nullptr)
            {
                BOOST_LOG_TRIVIAL(warning) << "Could not find session " << session;
                return sol::nil;
            }

            const auto& session_settings = state->session->get_settings();

            sol::table settings = lua.create_table();

            for (int i = lt::settings_pack::bool_type_base; i < lt::settings_pack::max_bool_setting_internal; i++)
            {
                const char *name = lt::name_for_setting(i);
                if (strcmp(name, "") == 0) continue;
                settings[name] = session_settings.get_bool(i);
            }

            for (int i = lt::settings_pack::int_type_base; i < lt::settings_pack::max_int_setting_internal; i++)
            {
                const char *name = lt::name_for_setting(i);
                if (strcmp(name, "") == 0) continue;
                settings[name] = session_settings.get_int(i);
            }

            for (int i = lt::settings_pack::string_type_base; i < lt::settings_pack::max_string_setting_internal; i++)
            {
                const char *name = lt::name_for_setting(i);
                if (strcmp(name, "") == 0) continue;
                settings[name] = session_settings.get_str(i);
            }

            return settings;
        };

        sessions["settings"]["set"] = [](sol::this_state s, const std::string& session, const sol::table& settings)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();
            const auto state = options.sessions.Get(session);

            if (state == nullptr)
            {
                BOOST_LOG_TRIVIAL(warning) << "Could not find session " << session;
                return;
            }

            lt::settings_pack session_settings = state->session->get_settings();

            for (const auto& [ key, value ] : settings)
            {
                const auto key_str = key.as<std::string>();

                if (porla::Sessions::DisallowedSetting(key_str))
                {
                    BOOST_LOG_TRIVIAL(warning) << "Will not set setting " << key_str;
                    continue;
                }

                const int type = lt::setting_by_name(key_str);

                if (type == -1)
                {
                    BOOST_LOG_TRIVIAL(warning) << "Unknown setting: " << key_str;
                    continue;
                }

                if ((type & lt::settings_pack::type_mask) == lt::settings_pack::bool_type_base)
                {
                    if (!value.is<bool>())
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key_str << " is not a boolean";
                        continue;
                    }

                    session_settings.set_bool(type, value.as<bool>());
                }
                else if((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base)
                {
                    if (!value.is<int>())
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key_str << " is not an integer";
                        continue;
                    }

                    session_settings.set_int(type, value.as<int>());
                }
                else if((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base)
                {
                    if (!value.is<std::string>())
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key_str << " is not a string";
                        continue;
                    }

                    session_settings.set_str(type, value.as<std::string>());
                }
            }

            state->session->apply_settings(session_settings);
        };

        return sessions;
    };
}
