#pragma once

#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "../methods/torrentspeersadd_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const json& j, porla::Methods::TorrentsPeersAddReq& req)
    {
        j.at("info_hash").get_to(req.info_hash);
        j.at("peers").get_to(req.peers);
    }

    static void to_json(json& j, const porla::Methods::TorrentsPeersAddRes& res)
    {
        j = json{
            {"foo", "bar"}
        };
    }
}
