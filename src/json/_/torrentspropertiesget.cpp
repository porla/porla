#include "../all.hpp"

#include "../../methods/torrents/torrentspropertiesget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
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
