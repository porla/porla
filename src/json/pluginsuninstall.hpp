#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsuninstall_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsUninstallReq,
        id)

    static void to_json(nlohmann::json& j, const PluginsUninstallRes& res)
    {
        j = {};
    }
}
