#include "../all.hpp"

#include "../../methods/plugins/pluginsget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetReq,
        id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetRes::Plugin,
        id,
        type,
        name,
        version,
        config,
        metadata,
        is_loaded)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetRes,
        plugin)
}
