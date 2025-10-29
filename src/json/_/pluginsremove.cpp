#include "../all.hpp"

#include "../../methods/plugins/pluginsremove_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsRemoveReq,
        id)

    void to_json(nlohmann::json& json, const PluginsRemoveRes& res)
    {
        json = {};
    }
}
