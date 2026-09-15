#pragma once

#include <map>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsUpdateReq
    {
        int                                   id;
        std::string                           name;
        bool                                  is_default;
        std::map<std::string, nlohmann::json> metadata;
    };

    struct SessionsUpdateRes
    {
    };
}
