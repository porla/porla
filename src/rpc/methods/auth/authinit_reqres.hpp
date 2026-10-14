#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Auth
{
    struct AuthInitReq
    {
        std::string username;
        std::string password;
    };

    PORLA_JSON_REQUEST(AuthInitReq, username, password)
}
