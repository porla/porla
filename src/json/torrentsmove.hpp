#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrents/torrentsmove_reqres.hpp"
#include "ltinfohash.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsMoveReq,
        flags,
        info_hash,
        path,
        session_id)

    static void to_json(json& j, const TorrentsMoveRes& res)
    {
        j = {};
    }
}
