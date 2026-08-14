#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentspropertiesset_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPropertiesSetReq,
        info_hash,
        session_id,
        download_limit,
        flags,
        flags_mask,
        max_connections,
        max_uploads,
        upload_limit)
}
