#pragma once

#include <libtorrent/info_hash.hpp>
#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "../methods/torrentspause_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const nlohmann::json& j, TorrentsPauseReq& req)
    {
        j.at("info_hash").get_to(req.info_hash);
    }

    static void to_json(nlohmann::json& j, const TorrentsPauseRes& res)
    {
        j = {};
    }
}
