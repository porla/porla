#pragma once

#include <filesystem>
#include <optional>

namespace porla::MediaInfo
{
    struct Container;

    class Parser
    {
    public:
        static std::optional<Container> Parse(const std::filesystem::path& path);
        static std::optional<Container> ParseExternal(const std::filesystem::path& path);
    };
}
