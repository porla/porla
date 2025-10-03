#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/presets/presetslist_reqres.hpp"

using json = nlohmann::json;

namespace porla::Methods::Presets
{
    static void from_json(const json& j, PresetsListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsListRes::ListItem,
        id,
        name);

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsListRes,
        presets);
}
