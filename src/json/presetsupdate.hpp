#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/presets/presetsupdate_reqres.hpp"

using json = nlohmann::json;

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
        session,
        save_path,
        storage_mode,
        tags,
        upload_limit);

    static void to_json(json& j, const PresetsUpdateRes& res)
    {
        j = {};
    }
}
