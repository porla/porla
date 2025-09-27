#pragma once

#include <nlohmann/json.hpp>

#include "../methods/presets/presetsadd_reqres.hpp"
#include "utils.hpp"

using json = nlohmann::json;

namespace porla::Methods::Presets
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsAddReq,
        name)

    static void to_json(json& j, const PresetsAddRes& res)
    {
        j = {};
    }
}
