#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsupdate_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsUpdateReq,
        name)

    static void to_json(nlohmann::json& j, const PluginsUpdateRes& res)
    {
        j = {};
    }
}
