#pragma once

#include <optional>
#include <string>

#include <libtorrent/info_hash.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsQueueAnyReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
    };

    struct TorrentsQueueAnyRes
    {
    };
}
