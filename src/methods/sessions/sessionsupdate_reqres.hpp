#pragma once

#include <map>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods::Sessions
{
    struct SessionsUpdateReq
    {
        int                                                  id;
        std::optional<std::string>                           name;
        std::optional<bool>                                  is_default;
        std::optional<std::map<std::string, nlohmann::json>> metadata;
        std::optional<std::map<std::string, nlohmann::json>> settings;
    };

    struct SessionsUpdateRes
    {
    };
}
