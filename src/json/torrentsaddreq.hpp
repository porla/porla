#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/torrentsaddreq.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsAddReq,
        magnet_uri,
        preset,
        save_path,
        ti,
        trackers)
}
