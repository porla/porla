#pragma once

#include <libtorrent/settings_pack.hpp>
#include <toml++/toml.h>

namespace porla
{
    class SettingsPack
    {
    public:
        static libtorrent::settings_pack Load(const toml::table& cfg);
    };
}
