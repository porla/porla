#include "../all.hpp"

#include "../../rpc/methods/plugins/pluginsinstall_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsInstallReq,
        owner,
        repository,
        version,
        config)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PluginsInstallRes,
        id)
}
