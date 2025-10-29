#include "../all.hpp"

#include "../../methods/torrents/torrentsresume_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
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
