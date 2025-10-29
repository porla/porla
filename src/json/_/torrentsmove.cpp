#include "../all.hpp"

#include "../../methods/torrents/torrentsmove_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsMoveReq,
        flags,
        info_hash,
        path,
        session_id)


    void to_json(nlohmann::json& json, const TorrentsMoveRes& res)
    {
        json = {};
    }
}
