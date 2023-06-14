#pragma once

#include <map>
#include <string>

#include "../config.hpp"

namespace porla::Methods
{
    struct PluginsUninstallReq
    {
        std::string name;
    };

    struct PluginsUninstallRes
    {
    };
}
