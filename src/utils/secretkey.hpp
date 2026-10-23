#pragma once

#include <filesystem>
#include <string>

namespace porla::Utils
{
    class SecretKey
    {
    public:
        static std::string Load(const std::filesystem::path& file, int length = 32);
    };
}
