#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/torrentsoverview_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const nlohmann::json& j, TorrentsOverviewReq& res)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsOverviewRes,
        torrents_per_category,
        torrents_per_session,
        torrents_per_tag,
        torrents_per_tracker);
}
