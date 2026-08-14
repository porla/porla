#pragma once

#include <optional>
#include <vector>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/peer_info.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsPeersListReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsPeersListRes
    {
        std::vector<libtorrent::peer_info> peers;
    };
}
