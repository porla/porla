#include "../all.hpp"

#include "../../rpc/methods/plugins/pluginsremove_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsRemoveReq,
        id)

    void to_json(nlohmann::json& json, const PluginsRemoveRes& res)
    {
        json = {};
    }
}
