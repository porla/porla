#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsrecheck_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsRecheckReq,
        info_hash,
        session_id)

    void to_json(nlohmann::json& json, const TorrentsRecheckRes& res)
    {
        json = {};
    }
}
