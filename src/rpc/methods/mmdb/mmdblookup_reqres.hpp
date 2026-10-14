#pragma once

#include <map>
#include <string>
#include <unordered_set>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Mmdb
{
    struct MmdbLookupReq
    {
        std::unordered_set<std::string> values;
    };

    struct MmdbLookupRes
    {
        std::map<std::string, nlohmann::json> results;
    };

    PORLA_JSON_REQUEST(MmdbLookupReq, values)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(MmdbLookupRes, results)
}
