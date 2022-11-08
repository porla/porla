#pragma once

#include <nlohmann/json.hpp>

#include "../methods/sessionresume_reqres.hpp"

namespace porla::Methods
{
    static void from_json(const nlohmann::json& j, SessionResumeReq& req)
    {
    }

    static void to_json(nlohmann::json& j, const SessionResumeRes& res)
    {
        j = true;
    }
}
