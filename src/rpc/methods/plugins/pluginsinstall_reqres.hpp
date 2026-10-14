#pragma once

#include <optional>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Plugins
{
    struct PluginsInstallReq
    {
        std::string                owner;
        std::string                repository;
        std::string                version;
        std::optional<std::string> config;
    };

    struct PluginsInstallRes
    {
        int id;
    };

    PORLA_JSON_REQUEST(PluginsInstallReq, owner, repository, version, config)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(PluginsInstallRes, id)
}
