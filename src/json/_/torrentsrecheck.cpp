#include "../all.hpp"

#include "../../methods/torrents/torrentsrecheck_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
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
