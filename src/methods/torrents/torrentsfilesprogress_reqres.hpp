#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsFilesProgressReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsFilesProgressRes
    {
        std::vector<std::int64_t> progress;
    };
}
