#pragma once

#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "lttorrentflags.hpp"
#include "utils.hpp"
#include "../methods/torrentspropertiesset_reqres.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPropertiesSetReq,
        info_hash,
        download_limit,
        flags,
        max_connections,
        max_uploads,
        upload_limit);
}
