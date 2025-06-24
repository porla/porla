#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct SessionsAddReq
    {
        std::string name;
        std::map<std::string, nlohmann::json> settings;
    };

    struct SessionsAddRes
    {
    };
}
