#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsconfigure_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsConfigureReq,
        id,
        config)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsConfigureRes,
        _)
}
