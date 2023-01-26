#pragma once

#include <nlohmann/json.hpp>

#include "lterrorcode.hpp"
#include "ltinfohash.hpp"
#include "utils.hpp"
#include "../methods/torrentslist_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListReq,
        filters,
        include_metadata,
        order_by,
        order_by_dir,
        page,
        page_size);

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListReqFilter,
        field,
        args);

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListRes::Item,
        all_time_download,
        all_time_upload,
        category,
        download_rate,
        error,
        eta,
        flags,
        info_hash,
        has_mediainfo,
        list_peers,
        list_seeds,
        metadata,
        moving_storage,
        name,
        num_peers,
        num_seeds,
        progress,
        queue_position,
        ratio,
        save_path,
        size,
        state,
        tags,
        total,
        total_done,
        upload_rate);

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListRes,
        order_by,
        order_by_dir,
        page,
        page_size,
        torrents,
        torrents_total);
}
