#include "../all.hpp"

#include "../../rpc/methods/plugins/pluginsget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetReq,
        id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetRes::Plugin,
        id,
        path,
        name,
        version,
        config,
        metadata,
        is_loaded)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsGetRes,
        plugin)
}
