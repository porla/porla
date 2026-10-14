#pragma once

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Auth
{
    struct AuthKeysRemoveReq
    {
        std::string id;
    };

    PORLA_JSON_REQUEST(AuthKeysRemoveReq, id)
}
