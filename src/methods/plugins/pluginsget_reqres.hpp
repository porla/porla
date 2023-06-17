#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace porla::Methods
{
    struct PluginsGetReq
    {
        std::string name;
    };

    struct PluginsGetRes
    {
        std::optional<std::string> config;
        std::filesystem::path      path;
        std::vector<std::string>   tags;
        std::optional<std::string> version;
    };
}
