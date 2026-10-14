#pragma once

#include <map>
#include <optional>
#include <string>

#include "../../../json/utils.hpp"

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

    PORLA_JSON_REQUEST(PluginsAddReq, path, config)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(PluginsAddRes, id)
}
