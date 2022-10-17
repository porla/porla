#pragma once

#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "../methods/torrentspeerslist_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const json& j, porla::Methods::TorrentsPeersListReq& req)
    {
    }

    static void to_json(json& j, const porla::Methods::TorrentsPeersListRes::PeerItem& peer)
    {
        j = {
            {"client", peer.client},
            {"dl_speed", peer.down_speed},
            {"ul_speed", peer.up_speed}
        };
    }

    static void to_json(json& j, const porla::Methods::TorrentsPeersListRes& res)
    {
        j = json{
            {"peers", res.peers}
        };
    }
}
