#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsmediainfo_reqres.hpp"

#include "ltinfohash.hpp"
#include "mediainfo.hpp"
#include "utils.hpp"

namespace porla::Methods
{
NLOHMANN_JSONIFY_ALL_THINGS(
    TorrentsMediaInfoReq,
    info_hash)

NLOHMANN_JSONIFY_ALL_THINGS(
    TorrentsMediaInfoRes,
    mediainfo)
}
