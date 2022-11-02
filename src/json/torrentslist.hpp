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
        download_rate,
        flags,
        info_hash,
        list_peers,
        list_seeds,
        name,
        num_peers,
        num_seeds,
        progress,
        queue_position,
        save_path,
        size,
        state,
        total,
        total_done,
        upload_rate)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListRes,
        page,
        page_size,
        torrents,
        torrents_total)
}
