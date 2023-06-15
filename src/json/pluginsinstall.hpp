#pragma once

#include <nlohmann/json.hpp>

#include "../methods/plugins/pluginsinstall_reqres.hpp"
#include "utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsInstallReq,
        config,
        enable,
        path)

    static void to_json(nlohmann::json& j, const PluginsInstallRes& res)
    {
        j = {};
    }
}
