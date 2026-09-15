#pragma once

#include <optional>

#include <libtorrent/info_hash.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsPropertiesGetReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsPropertiesGetRes
    {
        int download_limit;
        libtorrent::torrent_flags_t flags;
        int max_connections;
        int max_uploads;
        int upload_limit;
    };
}
