#include "../all.hpp"

#include "../../methods/torrents/torrentspropertiesset_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPropertiesSetReq,
        info_hash,
        session_id,
        download_limit,
        flags,
        max_connections,
        max_uploads,
        upload_limit)
}
