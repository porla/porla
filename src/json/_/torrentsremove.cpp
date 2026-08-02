#include "../all.hpp"

#include "../../methods/torrents/torrentsremovereq.hpp"
#include "../../methods/torrents/torrentsremoveres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsRemoveReq,
        info_hash,
        session_id,
        remove_data)

    void to_json(nlohmann::json& json, const TorrentsRemoveRes& res)
    {
    }
}
