#pragma once

#include <map>
#include <string>
#include <vector>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace porla::Methods::Mmdb
{
    struct MmdbLookupReq
    {
        std::unordered_set<std::string> values;
    };

    struct MmdbLookupRes
    {
        std::map<std::string, nlohmann::json> results;
    };
}
