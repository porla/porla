#include "../all.hpp"

#include "../../methods/torrents/torrentspeersadd_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPeersAddReq,
        info_hash,
        session_id,
        peers)

    void to_json(nlohmann::json& json, const TorrentsPeersAddRes& res)
    {
        json = true;
    }
}
