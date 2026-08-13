#include "../all.hpp"

#include "../../methods/plugins/pluginsinstall_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Plugins
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsInstallReq,
        owner,
        repository,
        version)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsInstallRes,
        id)
}
