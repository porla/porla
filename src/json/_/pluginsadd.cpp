#include "../all.hpp"

#include "../../methods/plugins/pluginsadd_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsAddReq,
        type,
        data,
        config,
        metadata)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsAddRes,
        id)
}
