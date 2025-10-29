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
        std::optional<std::map<std::string, nlohmann::json>> settings;
    };

    struct SessionsUpdateRes
    {
    };
}
