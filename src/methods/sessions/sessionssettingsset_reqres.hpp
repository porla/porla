#pragma once

#include <map>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods::Sessions
{
    struct SessionsSettingsSetReq
    {
        int                                   id;
        std::map<std::string, nlohmann::json> settings;
    };

    struct SessionsSettingsSetRes
    {
    };
}
