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
        struct VersionInfo
        {
            std::string head_name;
            std::string shorthand;
        };

        struct Plugin
        {
            bool                       can_configure;
            bool                       can_uninstall;
            std::string                name;
            fs::path                   path;
            std::optional<VersionInfo> version_info;
        };

        std::vector<Plugin> plugins;
    };
}
