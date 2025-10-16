#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"

#include "../methods/torrents/torrentstrackerslist_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsTrackersListReq,
        info_hash,
        session_id)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        TorrentsTrackersListRes,
        trackers)
}
