#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

namespace porla::Lua
{
    struct PluginSource
    {
        std::string                              entrypoint;
        std::map<std::string, std::vector<char>> sources;

        static std::optional<PluginSource> Load(const fs::path& path);
    };
}
