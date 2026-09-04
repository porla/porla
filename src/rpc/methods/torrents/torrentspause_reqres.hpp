#pragma once

#include <optional>

#include <libtorrent/info_hash.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsPauseReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsPauseRes {};
}
