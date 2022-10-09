#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsqueryreq.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void from_json(const json& j, porla::Methods::TorrentsQueryReq& req)
    {
        j.at("query").get_to(req.query);
    }
}
