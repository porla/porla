#include "../all.hpp"

#include "../../methods/plugins/pluginsupdate_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
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
