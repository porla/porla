#pragma once

#include <string>

namespace porla::Rpc::Methods::Auth
{
    struct AuthKeysCreateReq
    {
        std::string name;
    };

    struct AuthKeysCreateRes
    {
        std::string id;
        std::string secret;
    };
}
