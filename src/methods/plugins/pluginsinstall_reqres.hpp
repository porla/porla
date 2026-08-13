#pragma once

#include <string>

namespace porla::Methods::Plugins
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
