#pragma once

#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "../methods/torrentspeerslist_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const json& j, porla::Methods::TorrentsPeersListReq& req)
    {
        j.at("info_hash").get_to(req.info_hash);
    }

    static void to_json(json& j, const porla::Methods::TorrentsPeersListRes& res)
    {
        j = json{
            {"peers", res.peers}
        };
    }
}
