#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Rpc::Methods::Auth
{
    struct AuthInitReq
    {
        std::string username;
        std::string password;
    };

    struct AuthInitRes
    {
    };
}
