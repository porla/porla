#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct PluginsGetReq
    {
        int id;
    };

    struct PluginsGetRes
    {
        int                                                  id;
        std::string                                          type;
        std::optional<std::string>                           name;
        std::optional<std::string>                           version;
        std::optional<std::string>                           config;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
    };
}
