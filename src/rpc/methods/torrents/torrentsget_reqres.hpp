#pragma once

#include <optional>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_status.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsGetReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsGetRes
    {
        libtorrent::torrent_status torrent;
    };
}
