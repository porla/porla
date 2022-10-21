#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessionsettingslist_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        SessionSettingsListReq,
        keys)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        SessionSettingsListRes,
        settings)
}
