#include "../all.hpp"

#include "../../rpc/methods/plugins/pluginsupdate_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsUpdateReq,
        id,
        config,
        metadata,
        path)

    void to_json(nlohmann::json& json, const PluginsUpdateRes& res)
    {
        json = {};
    }
}
