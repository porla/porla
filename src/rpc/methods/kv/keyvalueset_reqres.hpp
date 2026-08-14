#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Rpc::Methods::Kv
{
    struct KeyValueSetReq
    {
        std::map<std::string, nlohmann::json> values;
    };

    struct KeyValueSetRes
    {
    };
}
