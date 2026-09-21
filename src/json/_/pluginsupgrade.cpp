#include "../all.hpp"

#include "../../rpc/methods/plugins/pluginsupgrade_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsUpgradeReq,
        id,
        version)

    void to_json(nlohmann::json& json, const PluginsUpgradeRes& res)
    {
        json = {};
    }
}
