#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/torrents/torrentsoverview_reqres.hpp"

namespace porla::Methods::Torrents
{
    static void from_json(const nlohmann::json& j, TorrentsOverviewReq& res)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsOverviewSession,
        torrents_per_category,
        torrents_per_state,
        torrents_per_tag,
        torrents_per_tracker,
        torrents_total);

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsOverviewRes,
        sessions);
}
