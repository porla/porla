#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct SessionsRemoveReq
    {
        std::string name;
    };

    struct SessionsRemoveRes
    {
    };
}
