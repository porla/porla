#include "../all.hpp"

#include "../../methods/torrents/torrentspeerslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPeersListReq,
        info_hash,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPeersListRes,
        peers)
}
