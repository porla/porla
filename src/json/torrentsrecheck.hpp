#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsrecheck_reqres.hpp"
#include "ltinfohash.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsRecheckReq,
        info_hash)

    static void to_json(json& j, const TorrentsRecheckRes& res)
    {
        j = {};
    }
}
