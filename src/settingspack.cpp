#include "settingspack.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/fingerprint.hpp>

namespace lt = libtorrent;
using porla::SettingsPack;

void SettingsPack::Apply(const toml::table& cfg, libtorrent::settings_pack& pack)
{
    for (int i = lt::settings_pack::bool_type_base; i < lt::settings_pack::max_bool_setting_internal; i++)
    {
        const char* name = lt::name_for_setting(i);
        if (strcmp(name, "") == 0) continue;

        if (auto val = cfg[name])
        {
            if (!val.is_boolean())
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "Invalid setting " << name << " - not a bool (" << val.type() << ")";
                continue;
            }

            int key = lt::setting_by_name(name);
            pack.set_bool(key, *val.value<bool>());
        }
    }

    for (int i = lt::settings_pack::int_type_base; i < lt::settings_pack::max_int_setting_internal; i++)
    {
        const char* name = lt::name_for_setting(i);
        if (strcmp(name, "") == 0) continue;

        if (auto val = cfg[name])
        {
            if (!val.is_number())
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "Invalid setting " << name << " - not a number (" << val.type() << ")";
                continue;
            }

            int key = lt::setting_by_name(name);
            pack.set_int(key, *val.value<int>());
        }
    }

    for (int i = lt::settings_pack::string_type_base; i < lt::settings_pack::max_string_setting_internal; i++)
    {
        const char* name = lt::name_for_setting(i);
        if (strcmp(name, "") == 0) continue;

        if (auto val = cfg[name])
        {
            if (!val.is_string())
            {
                BOOST_LOG_TRIVIAL(warning)
                    << "Invalid setting " << name << " - not a string (" << val.type() << ")";
                continue;
            }

            int key = lt::setting_by_name(name);
            pack.set_str(key, *val.value<std::string>());
        }
    }
}
