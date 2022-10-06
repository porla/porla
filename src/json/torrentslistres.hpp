#pragma once

#include <nlohmann/json.hpp>

#include "../methods/torrentslistres.hpp"

using json = nlohmann::json;

namespace porla::Methods
{
    static void to_json(json& j, const porla::Methods::TorrentsListRes& res)
    {
        j = json{
            {"foo", "bar"}
        };
    }
}
