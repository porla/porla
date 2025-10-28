#pragma once

#include <nlohmann/json.hpp>

#include "lterrorcode.hpp"
#include "ltinfohash.hpp"
#include "lttorrentstatus.hpp"
#include "utils.hpp"
#include "../methods/torrents/torrentslist_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListReq,
        filters,
        order_by,
        order_by_dir,
        page,
        page_size);

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListRes,
        order_by,
        order_by_dir,
        page,
        page_size,
        torrents,
        torrents_total,
        torrents_total_unfiltered);
}
