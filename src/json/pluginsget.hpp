#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsget_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetReq,
        name)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetRes,
        config,
        path,
        tags,
        version)
}
