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
        static void Apply(const toml::table& cfg, libtorrent::settings_pack& pack);
    };
}
