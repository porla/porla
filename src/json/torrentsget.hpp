#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrents/torrentsget_reqres.hpp"
#include "lttorrentstatus.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsGetReq,
        info_hash,
        session_id)

    static void to_json(nlohmann::json& j, const TorrentsGetRes& res)
    {
        j = {
            {"torrent", res.torrent}
        };
    }
}
