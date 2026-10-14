#pragma once

#include <optional>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsPauseReq
    {
        int id;
    };

    PORLA_JSON_REQUEST(SessionsPauseReq, id)
}
