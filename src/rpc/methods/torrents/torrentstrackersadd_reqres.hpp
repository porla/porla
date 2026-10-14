#pragma once

#include <optional>
#include <string>
#include <vector>

#include <libtorrent/info_hash.hpp>

#include "../../../json/lttypes.hpp"
#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    struct TorrentsTrackersAddReq
    {
        libtorrent::info_hash_t info_hash;
        std::optional<int>      session_id;
        std::string             url;
        std::optional<uint8_t>  tier;
    };

    PORLA_JSON_REQUEST(
        TorrentsTrackersAddReq,
        info_hash,
        session_id,
        url,
        tier)
}
