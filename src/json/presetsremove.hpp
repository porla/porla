#pragma once

#include <nlohmann/json.hpp>

#include "../methods/presets/presetsremove_reqres.hpp"
#include "utils.hpp"

using json = nlohmann::json;

namespace porla::Methods::Presets
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsRemoveReq,
        id)

    static void to_json(json& j, const PresetsRemoveRes& res)
    {
        j = {};
    }
}
