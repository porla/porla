#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessionpause_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const nlohmann::json& j, SessionPauseReq& req)
    {
    }

    static void to_json(nlohmann::json& j, const SessionPauseRes& res)
    {
    }
}
