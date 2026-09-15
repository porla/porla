#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentstrackersadd_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsTrackersAddReq,
        info_hash,
        session_id,
        url,
        tier)

    void to_json(nlohmann::json& j, const TorrentsTrackersAddRes& res)
    {
        j = {};
    }
}
