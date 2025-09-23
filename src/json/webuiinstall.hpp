#pragma once

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "../methods/webui/webuiinstall_reqres.hpp"

using json = nlohmann::json;

namespace porla::Methods::WebUI
{
    static void from_json(const json& j, WebUIInstallReq& req)
    {
        j["data"].get_to(req.data);
    }

    static void to_json(json& j, const WebUIInstallRes& res)
    {
        j = {};
    }
}
