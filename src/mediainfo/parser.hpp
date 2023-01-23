#pragma once

#include <filesystem>

namespace porla::MediaInfo
{
    class Parser
    {
    public:
        static bool Parse(const std::filesystem::path& path);
    };
}