#include "../all.hpp"

#include "../../methods/torrents/torrentsmetadatalist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsMetadataListReq,
        info_hash,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsMetadataListRes,
        metadata)
}
