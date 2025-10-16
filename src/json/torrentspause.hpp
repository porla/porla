#pragma once

#include <libtorrent/info_hash.hpp>
#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "utils.hpp"

#include "../methods/torrents/torrentspause_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPauseReq,
        info_hash,
        session_id)

    static void to_json(nlohmann::json& j, const TorrentsPauseRes& res)
    {
        j = {};
    }
}
