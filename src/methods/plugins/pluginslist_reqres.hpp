#pragma once

#include <optional>
#include <vector>

namespace porla::Methods
{
    struct PluginsListReq
    {
    };

    struct PluginsListRes
    {
        struct Plugin
        {
            int                        id;
            std::string                type;
            std::optional<std::string> name;
            std::optional<std::string> version;
        };

        std::vector<Plugin> plugins;
    };
}
