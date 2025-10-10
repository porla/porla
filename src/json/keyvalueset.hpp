#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/keyvalueset_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        KeyValueSetReq,
        values);

    static void to_json(nlohmann::json& j, const KeyValueSetRes& res)
    {
        j = {};
    }
}
