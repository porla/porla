#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Methods::Auth
{
    struct AuthLoginReq
    {
        std::string username;
        std::string password;
    };

    struct AuthLoginRes
    {
        std::string token;
    };
}
