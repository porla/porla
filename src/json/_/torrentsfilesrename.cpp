#include "../all.hpp"

#include "../../rpc/methods/torrents/torrentsfilesrename_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Torrents
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsFilesRenameReq,
        info_hash,
        session_id,
        file_index,
        file_path)

    void to_json(nlohmann::json& j, const TorrentsFilesRenameRes& res)
    {
        j = {};
    }
}
