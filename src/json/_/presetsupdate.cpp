#include "../all.hpp"

#include "../../methods/presets/presetsupdate_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Presets
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsUpdateReq,
        id,
        name,
        category,
        download_limit,
        max_connections,
        max_uploads,
        metadata,
        session_id,
        save_path,
        storage_mode,
        tags,
        upload_limit);

    void to_json(nlohmann::json& j, const PresetsUpdateRes& res)
    {
        j = {};
    }
}
