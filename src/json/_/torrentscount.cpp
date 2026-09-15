#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentscount_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsCountReq,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsCountRes,
        categories,
        downloading,
        downloading_queued,
        error,
        finished,
        paused,
        seeding,
        seeding_queued,
        total,
        tags,
        trackers)
}
