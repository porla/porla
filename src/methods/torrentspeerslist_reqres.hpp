#pragma once

#include <vector>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsPeersListReq
    {
        libtorrent::info_hash_t info_hash;
    };

    struct TorrentsPeersListRes
    {
        struct PeerItem
        {
            std::string client;
            int down_speed;
            int up_speed;
        };

        std::vector<PeerItem> peers;
    };
}
