#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct SessionSettingsListReq
    {
        std::optional<std::unordered_set<std::string>> keys;
    };

    struct SessionSettingsListRes
    {
        std::map<std::string, nlohmann::json> settings;
    };
}
