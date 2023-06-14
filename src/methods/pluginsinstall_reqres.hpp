#pragma once

#include <map>
#include <string>

#include "../config.hpp"

namespace porla::Methods
{
    struct PluginsInstallReq
    {
        std::optional<std::string> config;
        std::optional<bool>        enable;
        fs::path                   path;
    };

    struct PluginsInstallRes
    {
    };
}
