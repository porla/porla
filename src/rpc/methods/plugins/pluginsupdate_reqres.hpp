#pragma once

#include <map>
#include <optional>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsUpdateReq
    {
        int                        id;
        std::string                path;
        std::optional<std::string> config;
    };

    PORLA_JSON_REQUEST(PluginsUpdateReq, id, path, config)
}
