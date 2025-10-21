#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/torrents/torrentsoverview_reqres.hpp"

namespace porla::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsOverviewReq,
        filters)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsOverviewSession,
        session_id,
        session_name,
        torrents_errors,
        torrents_per_category,
        torrents_per_flags,
        torrents_per_state,
        torrents_per_tag,
        torrents_per_tracker,
        torrents_total);

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsOverviewRes,
        sessions);
}
