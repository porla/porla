#pragma once

#include <optional>
#include <string>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_handle.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsQueueSetReq
    {
        libtorrent::info_hash_t      info_hash;
        std::optional<int>           session_id;
        libtorrent::queue_position_t queue_position;
    };

    struct TorrentsQueueSetRes
    {
    };
}
