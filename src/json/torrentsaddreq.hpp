#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/torrentsaddreq.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsAddReq,
        category,
        download_limit,
        http_seeds,
        magnet_uri,
        max_connections,
        max_uploads,
        metadata,
        name,
        preset,
        save_path,
        tags,
        ti,
        trackers,
        upload_limit,
        url_seeds)
}
