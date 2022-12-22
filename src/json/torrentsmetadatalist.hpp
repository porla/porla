#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsmetadatalist_reqres.hpp"

#include "ltinfohash.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsMetadataListReq,
        info_hash);

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsMetadataListRes,
        metadata);
}
