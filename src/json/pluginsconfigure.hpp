#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsconfigure_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsConfigureReq,
        config,
        name)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsConfigureRes,
        _)
}
