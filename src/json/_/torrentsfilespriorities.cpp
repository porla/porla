#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsfilespriorities_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesPrioritiesReq,
        info_hash,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesPrioritiesRes,
        priorities)
}
