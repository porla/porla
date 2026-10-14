#pragma once

#include <optional>
#include <string>

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsResumeReq
    {
        int id;
    };

    PORLA_JSON_REQUEST(SessionsResumeReq, id)
}
