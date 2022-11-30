#pragma once

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsPropertiesGetReq
    {
        libtorrent::info_hash_t info_hash;
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
