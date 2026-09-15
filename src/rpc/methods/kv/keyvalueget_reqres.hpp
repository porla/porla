#pragma once

#include <map>
#include <string>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace porla::Rpc::Methods::Kv
{
    struct KeyValueGetReq
    {
        std::unordered_set<std::string> keys;
    };

    struct KeyValueGetRes
    {
        std::map<std::string, nlohmann::json> values;
    };
}
