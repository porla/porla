#include "../all.hpp"

#include "../../methods/presets/presetsget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Presets
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsGetReq,
        id);

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsGetRes::Preset,
        id,
        name,
        is_default,
        metadata,
        category,
        download_limit,
        max_connections,
        max_uploads,
        session_id,
        save_path,
        storage_mode,
        tags,
        upload_limit);

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsGetRes,
        preset);
}
