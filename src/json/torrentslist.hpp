#pragma once

#include <nlohmann/json.hpp>

#include "ltinfohash.hpp"
#include "utils.hpp"
#include "../methods/torrentslist_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListReq,
        order_by,
        order_by_dir,
        page,
        page_size)

    static void to_json(json& j, const porla::Methods::TorrentsListRes::Item& item)
    {
        j = {
            {"name", item.name},
            {"queue_pos", item.queue_pos}
        };
    }

    static void to_json(json& j, const porla::Methods::TorrentsListRes& res)
    {
        j = {
            {"torrents", res.torrents},
            {"torrents_total", res.torrents_total}
        };
    }
}
