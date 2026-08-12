#include "../all.hpp"

#include "../../methods/webui/webuiinstall_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::WebUI
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        WebUIInstallReq,
        version)

    void to_json(nlohmann::json& json, const WebUIInstallRes& res)
    {
    }
}
