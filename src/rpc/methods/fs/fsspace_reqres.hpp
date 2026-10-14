#pragma once

#include <map>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Fs
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

    PORLA_JSON_REQUEST(FsSpaceReq, path)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
        FsSpaceQuota,
        blocks_limit_hard,
        blocks_limit_soft,
        blocks_time,
        current_inodes,
        current_space,
        inodes_limit_hard,
        inodes_limit_soft,
        inodes_time)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(FsSpaceRes, available, capacity, free, quota)
}
