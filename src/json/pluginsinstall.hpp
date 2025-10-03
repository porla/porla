#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsinstall_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsInstallReq,
        type,
        data,
        config,
        metadata)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsInstallRes,
        id)
}
