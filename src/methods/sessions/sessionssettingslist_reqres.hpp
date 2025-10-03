#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include <libtorrent/settings_pack.hpp>

namespace porla::Methods
{
    struct SessionsSettingsListReq
    {
        int                                            id;
        std::optional<std::unordered_set<std::string>> keys;
    };

    struct SessionsSettingsListRes
    {
        libtorrent::settings_pack settings;
    };
}
