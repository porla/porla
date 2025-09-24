#pragma once

#include <map>
#include <optional>
#include <string>

#include <libtorrent/settings_pack.hpp>
#include <nlohmann/json.hpp>

namespace porla::Methods::Sessions
{
    struct SessionsAddReq
    {
        std::string name;
        std::optional<std::string> settings_base;
        std::optional<std::map<std::string, nlohmann::json>> settings;
    };

    struct SessionsAddRes
    {
    };
}
