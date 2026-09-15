#pragma once

#include <optional>
#include <string>

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsPauseReq
    {
        int id;
    };

    struct SessionsPauseRes {};
}
