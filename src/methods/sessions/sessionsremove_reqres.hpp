#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods::Sessions
{
    struct SessionsRemoveReq
    {
        int id;
    };

    struct SessionsRemoveRes
    {
    };
}
