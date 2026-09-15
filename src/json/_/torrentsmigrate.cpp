#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsmigrate_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsMigrateReq,
        info_hash,
        session_id,
        target_session_id)


    void to_json(nlohmann::json& json, const TorrentsMigrateRes& res)
    {
        json = {};
    }
}
