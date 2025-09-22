#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace porla
{
    class WebUI
    {
    public:
        static void Download(const std::string& repo, const fs::path& target_file);
    };
}
