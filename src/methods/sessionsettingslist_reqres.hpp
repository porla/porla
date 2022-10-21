#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct SessionSettingsListReq
    {
        std::vector<std::string> keys;
    };

    struct SessionSettingsListRes
    {
        std::map<std::string, nlohmann::json> settings;
    };
}
