#pragma once

#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "utils.hpp"

#include "../methods/torrents/torrentspeersadd_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        porla::Methods::TorrentsPeersAddReq,
        info_hash,
        peers,
        session_id)

    static void to_json(json& j, const porla::Methods::TorrentsPeersAddRes& res)
    {
        j = true;
    }
}
