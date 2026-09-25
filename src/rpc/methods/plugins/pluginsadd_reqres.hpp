#pragma once

#include <map>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsAddReq
    {
        std::string                path;
        std::optional<std::string> config;
    };

    struct PluginsAddRes
    {
        int id;
    };
}
