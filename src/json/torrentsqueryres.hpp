#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsqueryres.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void to_json(json& j, const porla::Methods::TorrentsQueryRes& res)
    {
        j = json{
            {"results", res.results}
        };
    }
}
