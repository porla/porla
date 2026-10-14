#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsListReq
    {
    };

    struct PluginsListRes
    {
        struct Plugin
        {
            int                                   id;
            std::string                           path;
            std::optional<std::string>            name;
            std::optional<std::string>            version;
            std::map<std::string, nlohmann::json> metadata;
            bool                                  is_loaded;
        };

        std::vector<Plugin> plugins;
    };

    PORLA_JSON_NO_PARAMS(PluginsListReq)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        PluginsListRes::Plugin,
        id,
        path,
        name,
        version,
        metadata,
        is_loaded)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(PluginsListRes, plugins)
}
