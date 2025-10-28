#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrents/torrentsfilesprogress_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesProgressReq,
        info_hash,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesProgressRes,
        progress)
}
