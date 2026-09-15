#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsqueueset_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsQueueSetReq,
        info_hash,
        session_id,
        queue_position)

    void to_json(nlohmann::json& json, const TorrentsQueueSetRes& res)
    {
        json = {};
    }
}
