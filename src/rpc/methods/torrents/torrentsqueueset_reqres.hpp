#pragma once

#include <optional>
#include <string>

#include <libtorrent/info_hash.hpp>
#include <libtorrent/torrent_handle.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsQueueSetReq
    {
        libtorrent::info_hash_t      info_hash;
        std::optional<int>           session_id;
        libtorrent::queue_position_t queue_position;
    };

    PORLA_JSON_REQUEST(
        TorrentsQueueSetReq,
        info_hash,
        session_id,
        queue_position)
}
