#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessions/sessionspause_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsPauseReq,
        id)

    static void to_json(nlohmann::json& j, const SessionsPauseRes& res)
    {
    }
}
