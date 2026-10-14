#pragma once

#include <string>

#include "../../../json/utils.hpp"

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

    PORLA_JSON_REQUEST(AuthLoginReq, username, password)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(AuthLoginRes, token)
}
