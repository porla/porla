#pragma once

#include <vector>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/peer_info.hpp>

namespace porla::Methods
{
    struct TorrentsPeersListReq
    {
        libtorrent::info_hash_t info_hash;
    };

    struct TorrentsPeersListRes
    {
        std::vector<libtorrent::peer_info> peers;
    };
}
