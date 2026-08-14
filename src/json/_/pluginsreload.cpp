#include "../all.hpp"

#include "../../rpc/methods/plugins/pluginsreload_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsReloadReq,
        id)

    void to_json(nlohmann::json& json, const PluginsReloadRes& res)
    {
        json = {};
    }
}
