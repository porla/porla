#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsremove_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsRemoveReq,
        id)

    static void to_json(nlohmann::json& j, const PluginsRemoveRes& res)
    {
        j = {};
    }
}
