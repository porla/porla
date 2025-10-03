#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsget_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetReq,
        id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetRes,
        id,
        type,
        name,
        version,
        config,
        metadata)
}
