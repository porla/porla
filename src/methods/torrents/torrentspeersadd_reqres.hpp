#pragma once

#include <string>

#include <libtorrent/info_hash.hpp>

namespace porla::Methods
{
    struct TorrentsPeersAddReq
    {
        libtorrent::info_hash_t info_hash;
        std::vector<std::tuple<std::string, uint16_t>> peers;
    };

    struct TorrentsPeersAddRes {};
}
