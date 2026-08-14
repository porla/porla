#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsresume_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsResumeReq,
        info_hash,
        session_id)

    void to_json(nlohmann::json& json, const TorrentsResumeRes& res)
    {
        json = {};
    }
}
