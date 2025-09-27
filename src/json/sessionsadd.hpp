#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessions/sessionsadd_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods::Sessions
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsAddReq,
        name,
        settings,
        settings_base)

    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsAddRes,
        id);
}
