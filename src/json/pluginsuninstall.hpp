#pragma once

#include <nlohmann/json.hpp>

#include "../methods/pluginsuninstall_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsUninstallReq,
        name)

    static void to_json(nlohmann::json& j, const PluginsUninstallRes& res)
    {
        j = {};
    }
}
