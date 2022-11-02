#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsquery_reqres.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void from_json(const json& j, porla::Methods::TorrentsQueryReq& req)
    {
        j.at("query").get_to(req.query);
    }

    static void to_json(json& j, const porla::Methods::TorrentsQueryRes& res)
    {
        j = json{
            {"results", res.results}
        };
    }
}
