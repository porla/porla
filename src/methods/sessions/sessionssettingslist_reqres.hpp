#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct SessionsSettingsListReq
    {
        std::optional<std::string>                     name;
        std::optional<std::unordered_set<std::string>> keys;
    };

    struct SessionsSettingsListRes
    {
        std::map<std::string, nlohmann::json> settings;
    };
}
