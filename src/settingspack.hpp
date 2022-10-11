#pragma once

#include <map>
#include <string>

#include <libtorrent/settings_pack.hpp>
#include <toml++/toml.h>

namespace porla
{
    class SettingsPack
    {
    public:
        static libtorrent::settings_pack Load(const toml::table& cfg);

    private:
        static std::map<std::string, int> AllowedStringSettings();
        static std::map<std::string, int> AllowedBoolSettings();
        static std::map<std::string, int> AllowedIntSettings();
    };
}
