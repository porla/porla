#include "../all.hpp"

#include "../../methods/torrents/torrentsaddreq.hpp"
#include "../../methods/torrents/torrentsaddres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsAddReq,
        category,
        download_limit,
        flags,
        http_seeds,
        magnet_uri,
        max_connections,
        max_uploads,
        metadata,
        name,
        preset_id,
        save_path,
        session_id,
        tags,
        ti,
        trackers,
        upload_limit,
        url_seeds)

    void to_json(nlohmann::json& json, const TorrentsAddRes& res)
    {
        json = {
            {"info_hash", res.info_hash},
            {"session_id", res.session_id}
        };
    }
}
