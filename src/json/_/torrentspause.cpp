#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentspause_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPauseReq,
        info_hash,
        session_id)

    void to_json(nlohmann::json& json, const TorrentsPauseRes& res)
    {
        json = {};
    }
}
