#include "../all.hpp"

#include "../../methods/webui/webuiinstall_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::WebUI
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        WebUIInstallReq,
        data)

    void to_json(nlohmann::json& j, const WebUIInstallRes& res)
    {
        j = {};
    }
}
