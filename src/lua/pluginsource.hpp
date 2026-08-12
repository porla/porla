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
        fs::path                              entrypoint;
        std::map<fs::path, std::vector<char>> sources;

        static std::optional<PluginSource> Load(const fs::path& path);
    };
}
