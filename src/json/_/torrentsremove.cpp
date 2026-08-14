#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsremovereq.hpp"
#include "../../rpc/methods/torrents/torrentsremoveres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
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
