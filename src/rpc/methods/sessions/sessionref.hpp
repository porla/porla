#pragma once

#include "../../../json/utils.hpp"

namespace porla::Rpc::Methods::Sessions
{
    struct SessionRef
    {
        int id;
    };

    PORLA_JSON_REQUEST(SessionRef, id)
}
