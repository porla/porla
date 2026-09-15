#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListFilters,
        category,
        errc,
        flags,
        query,
        session_id,
        save_path,
        state,
        status,
        tags)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListReq,
        filters,
        order_by,
        order_by_dir,
        page,
        page_size)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsListRes,
        order_by,
        order_by_dir,
        page,
        page_size,
        torrents,
        torrents_total,
        torrents_total_unfiltered)
}
