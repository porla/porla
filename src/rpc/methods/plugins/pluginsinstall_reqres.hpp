#pragma once

#include <optional>
#include <string>

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsInstallReq
    {
        std::string                owner;
        std::string                repository;
        std::string                version;
        std::optional<std::string> config;
    };

    struct PluginsInstallRes
    {
        int id;
    };
}
