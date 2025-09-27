#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessions/sessionssettingslist_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionsSettingsListReq,
        id,
        keys)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        SessionsSettingsListRes,
        settings)
}
