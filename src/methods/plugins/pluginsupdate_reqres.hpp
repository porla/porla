#pragma once

#include <map>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct PluginsUpdateReq
    {
        int                                                  id;
        std::optional<std::string>                           config;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
        std::optional<std::string>                           data;
    };

    struct PluginsUpdateRes
    {
    };
}
