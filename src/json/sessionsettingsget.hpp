#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessionsettingslist_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        SessionSettingsListReq,
        keys)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        SessionSettingsListRes,
        settings)
}
