#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/presets/presetsget_reqres.hpp"

using json = nlohmann::json;

namespace porla::Methods::Presets
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsGetReq,
        id);

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsGetRes,
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
}
