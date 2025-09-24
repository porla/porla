#pragma once

#include <libtorrent/settings_pack.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace libtorrent
{
    static void from_json(const json& j, libtorrent::settings_pack& settings)
    {
        settings = lt::default_settings();

        const auto& input = j.get<std::map<std::string, json>>();

        for (const auto& [ key, value ] : input)
        {
            const int type = lt::setting_by_name(key);

            if (type == -1)
            {
                continue;
            }

            if ((type & lt::settings_pack::type_mask) == lt::settings_pack::bool_type_base)
            {
                if (!value.is_boolean())
                {
                    continue;
                }

                settings.set_bool(type, value.get<bool>());
            }
            else if((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base)
            {
                if (!value.is_number())
                {
                    continue;
                }

                settings.set_int(type, value.get<int>());
            }
            else if((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base)
            {
                if (!value.is_string())
                {
                    continue;
                }

                settings.set_str(type, value.get<std::string>());
            }
        }
    }

    static void to_json(json& j, const libtorrent::settings_pack& settings)
    {
        j = json::object();

        for (int i = lt::settings_pack::bool_type_base; i < lt::settings_pack::max_bool_setting_internal; i++)
        {
            const char *name = lt::name_for_setting(i);
            if (strcmp(name, "") == 0) continue;

            j[name] = settings.get_bool(i);
        }

        for (int i = lt::settings_pack::int_type_base; i < lt::settings_pack::max_int_setting_internal; i++)
        {
            const char *name = lt::name_for_setting(i);
            if (strcmp(name, "") == 0) continue;

            j[name] = settings.get_int(i);
        }

        for (int i = lt::settings_pack::string_type_base; i < lt::settings_pack::max_string_setting_internal; i++)
        {
            const char *name = lt::name_for_setting(i);
            if (strcmp(name, "") == 0) continue;

            j[name] = settings.get_str(i);
        }
    }
}
