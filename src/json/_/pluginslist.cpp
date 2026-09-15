#include "../all.hpp"

#include "../../rpc/methods/plugins/pluginslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    void from_json(const nlohmann::json& json, PluginsListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsListRes::Plugin,
        id,
        path,
        name,
        version,
        metadata,
        is_loaded)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsListRes,
        plugins)
}
