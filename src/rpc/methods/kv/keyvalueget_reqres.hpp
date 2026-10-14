#pragma once

#include <map>
#include <string>
#include <unordered_set>

#include "../../../json/utils.hpp"

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

    PORLA_JSON_REQUEST(KeyValueGetReq, keys)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(KeyValueGetRes, values)
}
