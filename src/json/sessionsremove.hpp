#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessions/sessionsremove_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsRemoveReq,
        id)

    static void to_json(nlohmann::json& j, const SessionsRemoveRes& res)
    {
        j = {};
    }
}
