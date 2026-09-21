#pragma once

#include <map>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsUpgradeReq
    {
        int         id;
        std::string version;
    };

    struct PluginsUpgradeRes
    {
    };
}
