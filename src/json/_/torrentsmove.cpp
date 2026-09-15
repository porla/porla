#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsmove_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
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
