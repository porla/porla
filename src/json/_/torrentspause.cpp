#include "../all.hpp"

#include "../../methods/torrents/torrentspause_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
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
