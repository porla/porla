#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrents/torrentstrackerslist_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        TorrentsTrackersListReq,
        info_hash)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        TorrentsTrackersListRes,
        trackers)
}
