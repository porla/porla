#include "../all.hpp"

#include "../../methods/torrents/torrentscount_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Torrents
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
