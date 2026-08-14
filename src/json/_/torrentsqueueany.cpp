#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsqueueany_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsQueueAnyReq,
        info_hash,
        session_id)

    void to_json(nlohmann::json& json, const TorrentsQueueAnyRes& res)
    {
        json = {};
    }
}
