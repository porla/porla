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
        PluginsListRes::VersionInfo,
        head_name,
        shorthand)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsListRes::Plugin,
        can_configure,
        can_uninstall,
        name,
        path,
        version_info)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsListRes,
        plugins)
}
