#pragma once

#include "../methods/torrentspropertiesget_reqres.hpp"

#include "ltinfohash.hpp"
#include "lttorrentflags.hpp"
#include "utils.hpp"

namespace porla::Methods
{
NLOHMANN_JSONIFY_ALL_THINGS(
    TorrentsPropertiesGetReq,
    info_hash)

NLOHMANN_JSONIFY_ALL_THINGS(
    TorrentsPropertiesGetRes,
    download_limit,
    flags,
    max_connections,
    max_uploads,
    upload_limit)
}