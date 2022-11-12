#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct SessionSettingsUpdateReq
    {
        std::map<std::string, nlohmann::json> settings;
    };

    struct SessionSettingsUpdateRes
    {
    };
}
