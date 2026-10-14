#pragma once

#include <map>
#include <optional>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsUpgradeReq
    {
        int         id;
        std::string version;
    };

    PORLA_JSON_REQUEST(PluginsUpgradeReq, id, version)
}
