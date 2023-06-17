#pragma once

#include <optional>
#include <string>
#include <vector>

namespace porla::Methods
{
    struct PluginsConfigureReq
    {
        std::string                name;
        std::optional<std::string> config;
    };

    struct PluginsConfigureRes
    {
        int _;
    };
}
