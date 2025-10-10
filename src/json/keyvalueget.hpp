#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/keyvalueget_reqres.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        KeyValueGetReq,
        keys);

    NLOHMANN_JSONIFY_ALL_THINGS(
        KeyValueGetRes,
        values);
}
