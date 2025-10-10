#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct KeyValueSetReq
    {
        std::map<std::string, nlohmann::json> values;
    };

    struct KeyValueSetRes
    {
    };
}
