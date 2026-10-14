#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsGetReq
    {
        int id;
    };

    struct PluginsGetRes
    {
        struct Plugin
        {
            int                                   id;
            std::string                           path;
            std::optional<std::string>            name;
            std::optional<std::string>            version;
            std::optional<std::string>            config;
            std::map<std::string, nlohmann::json> metadata;
            bool                                  is_loaded;
        };

        Plugin plugin;
    };

    PORLA_JSON_REQUEST(PluginsGetReq, id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        PluginsGetRes::Plugin,
        id,
        path,
        name,
        version,
        config,
        metadata,
        is_loaded)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(PluginsGetRes, plugin)
}
