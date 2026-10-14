#pragma once

#include <map>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsRemoveReq
    {
        int id;
    };

    PORLA_JSON_REQUEST(SessionsRemoveReq, id)
}
