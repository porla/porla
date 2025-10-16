#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct PluginsAddReq
    {
        std::string                type;
        std::string                data;

        std::optional<std::string>                           config;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
    };

    struct PluginsAddRes
    {
        int id;
    };
}
