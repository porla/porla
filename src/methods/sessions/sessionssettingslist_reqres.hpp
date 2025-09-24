#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include <libtorrent/settings_pack.hpp>

namespace porla::Methods
{
    struct SessionsSettingsListReq
    {
        std::optional<std::string>                     name;
        std::optional<std::unordered_set<std::string>> keys;
    };

    struct SessionsSettingsListRes
    {
        libtorrent::settings_pack settings;
    };
}
