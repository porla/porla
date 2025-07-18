#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessions/sessionsadd_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsAddReq,
        name,
        settings)

    static void to_json(nlohmann::json& j, const SessionsAddRes& res)
    {
        j = {};
    }
}
