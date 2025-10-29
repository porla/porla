#include "../all.hpp"

#include "../../methods/torrents/torrentstrackerslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsTrackersListReq,
        info_hash,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsTrackersListRes,
        trackers)
}
