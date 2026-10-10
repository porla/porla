#include "../all.hpp"

#include <libtorrent/settings_pack.hpp>

#include "../../utils/ltsettings.hpp"

namespace libtorrent
{
    void from_json(const nlohmann::json& j, settings_pack& settings)
    {
        settings = lt::settings_pack();

        porla::Utils::LibtorrentSettingsPack::Update(
            settings, j.get<std::map<std::string, nlohmann::json>>());
    }

    void to_json(nlohmann::json& j, const settings_pack& settings)
    {
        j = nlohmann::json::object();

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
