#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsfilesprioritize_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesPrioritizeReq::FilePrio,
        index,
        priority)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesPrioritizeReq,
        info_hash,
        session_id,
        priorities)

    void to_json(nlohmann::json& j, const TorrentsFilesPrioritizeRes& res)
    {
        j = {};
    }
}
