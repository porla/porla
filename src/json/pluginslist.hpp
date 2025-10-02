#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginslist_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    static void from_json(const nlohmann::json & j, PluginsListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsListRes::Plugin,
        id,
        name,
        version)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsListRes,
        plugins)
}
