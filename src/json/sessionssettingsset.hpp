#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessions/sessionssettingsset_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsSettingsSetReq,
        name,
        settings)

    static void to_json(nlohmann::json& j, const SessionsSettingsSetRes& res)
    {
        j = {};
    }
}
