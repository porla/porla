#pragma once

#include <libtorrent/info_hash.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsAddRes
    {
        libtorrent::info_hash_t info_hash;
        int                     session_id;
    };
}
