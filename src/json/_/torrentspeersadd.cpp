#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentspeersadd_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
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
