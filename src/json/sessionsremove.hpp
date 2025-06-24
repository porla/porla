#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessions/sessionsremove_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsRemoveReq,
        name)

    static void to_json(nlohmann::json& j, const SessionsRemoveRes& res)
    {
        j = {};
    }
}
