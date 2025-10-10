#pragma once

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsPauseReq
    {
        libtorrent::info_hash_t info_hash;
    };

    struct TorrentsPauseRes {};
}
