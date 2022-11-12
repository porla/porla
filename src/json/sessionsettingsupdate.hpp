#pragma once

#include "../methods/sessionsettingsupdate_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        SessionSettingsUpdateReq,
        settings)

    static void to_json(nlohmann::json& j, const SessionSettingsUpdateRes& res)
    {
        j = true;
    }
}
