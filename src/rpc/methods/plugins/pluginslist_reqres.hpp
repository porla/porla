#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

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
}
