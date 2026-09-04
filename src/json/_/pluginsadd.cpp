#include "../all.hpp"

#include "../../rpc/methods/plugins/pluginsadd_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsAddReq,
        path,
        config,
        metadata)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsAddRes,
        id)
}
