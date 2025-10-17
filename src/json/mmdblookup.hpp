#pragma once

#include <nlohmann/json.hpp>

#include "../methods/mmdblookup_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        MmdbLookupReq,
        values)

    NLOHMANN_JSONIFY_ALL_THINGS(
        MmdbLookupRes,
        results)
}
