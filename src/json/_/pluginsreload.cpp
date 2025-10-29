#include "../all.hpp"

#include "../../methods/plugins/pluginsreload_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsReloadReq,
        id)

    void to_json(nlohmann::json& json, const PluginsReloadRes& res)
    {
        json = {};
    }
}
