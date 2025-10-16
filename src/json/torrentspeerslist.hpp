#pragma once

#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "utils.hpp"

#include "../methods/torrents/torrentspeerslist_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        porla::Methods::TorrentsPeersListReq,
        info_hash,
        session_id)

    static void to_json(json& j, const porla::Methods::TorrentsPeersListRes& res)
    {
        j = json{
            {"peers", res.peers}
        };
    }
}
