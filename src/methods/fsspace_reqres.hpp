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

    struct FsSpaceQuota
    {
        std::uint64_t blocks_limit_hard;
        std::uint64_t blocks_limit_soft;
        std::uint64_t blocks_time;
        std::uint64_t current_inodes;
        std::uint64_t current_space;
        std::uint64_t inodes_limit_hard;
        std::uint64_t inodes_limit_soft;
        std::uint64_t inodes_time;

    };

    struct FsSpaceRes
    {
        std::uint64_t               available{0};
        std::uint64_t               capacity{0};
        std::uint64_t               free{0};
        std::optional<FsSpaceQuota> quota{std::nullopt};
    };
}
