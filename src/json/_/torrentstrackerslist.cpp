#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentstrackerslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsTrackersListReq,
        info_hash,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsTrackersListRes,
        trackers)
}
