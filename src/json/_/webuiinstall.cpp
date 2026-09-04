#include "../all.hpp"

#include "../../rpc/methods/webui/webuiinstall_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::WebUI
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        WebUIInstallReq,
        version)

    void to_json(nlohmann::json& json, const WebUIInstallRes& res)
    {
    }
}
