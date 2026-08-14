#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentspropertiesget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPropertiesGetReq,
        info_hash,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPropertiesGetRes,
        download_limit,
        flags,
        max_connections,
        max_uploads,
        upload_limit)
}
