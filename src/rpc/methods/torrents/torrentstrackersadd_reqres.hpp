#pragma once

#include <optional>
#include <string>
#include <vector>

#include <libtorrent/info_hash.hpp>

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsTrackersAddReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
        std::string             url;
        std::optional<uint8_t>  tier;
    };

    struct TorrentsTrackersAddRes
    {
    };
}
