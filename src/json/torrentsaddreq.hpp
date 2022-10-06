#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsaddreq.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void from_json(const json& json, porla::Methods::TorrentsAddReq& req)
    {
    }
}
