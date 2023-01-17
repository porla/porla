#pragma once

#include <map>
#include <string>

#include "../config.hpp"

namespace porla::Methods
{
    struct FsSpaceReq
    {
        std::string path;
    };

    struct FsSpaceRes
    {
        std::uint64_t available;
        std::uint64_t capacity;
        std::uint64_t free;
    };
}
