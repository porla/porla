#pragma once

#include <string>

namespace porla::Rpc::Methods::Auth
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
