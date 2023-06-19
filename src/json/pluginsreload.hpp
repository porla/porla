#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsreload_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsReloadReq,
        name)

    static void to_json(nlohmann::json& j, const PluginsReloadRes& res)
    {
        j = {};
    }
}
