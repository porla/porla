#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentsaddres.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void to_json(json& j, const porla::Methods::TorrentsAddRes& res)
    {
        j = {
            {"info_hash", res.info_hash}
        };
    }
}
