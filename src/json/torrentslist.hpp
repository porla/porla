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

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListRes::Item,
        info_hashes,
        list_peers,
        list_seeds,
        name,
        queue_pos,
        save_path)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListRes,
        torrents,
        torrents_total)
}
