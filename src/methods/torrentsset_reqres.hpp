#pragma once

#include <optional>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsSetReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int> download_limit;
        std::optional<libtorrent::torrent_flags_t> set_flags;
        std::optional<libtorrent::torrent_flags_t> unset_flags;
        std::optional<int> max_connections;
        std::optional<int> max_uploads;
        std::optional<int> upload_limit;
    };

    struct TorrentsSetRes
    {
    };
}
