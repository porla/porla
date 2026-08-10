#include "../all.hpp"

#include "../../methods/plugins/pluginslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
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
