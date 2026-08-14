#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsfilesprogress_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesProgressReq,
        info_hash,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesProgressRes,
        progress)
}
