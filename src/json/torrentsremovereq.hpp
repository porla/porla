#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsremovereq.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void from_json(const json& j, porla::Methods::TorrentsRemoveReq& req)
    {
    }
}
