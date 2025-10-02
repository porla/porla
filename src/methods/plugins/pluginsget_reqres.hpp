#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace porla::Methods
{
    struct PluginsGetReq
    {
        int id;
    };

    struct PluginsGetRes
    {
        std::optional<std::string> config;
        std::filesystem::path      path;
    };
}
