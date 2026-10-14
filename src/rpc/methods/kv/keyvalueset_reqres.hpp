#pragma once

#include <map>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Kv
{
    struct KeyValueSetReq
    {
        std::map<std::string, nlohmann::json> values;
    };

    PORLA_JSON_REQUEST(KeyValueSetReq, values)
}
