#pragma once

#include <string>

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsInstallReq
    {
        std::string owner;
        std::string repository;
        std::string version;
    };

    struct PluginsInstallRes
    {
        int id;
    };
}
