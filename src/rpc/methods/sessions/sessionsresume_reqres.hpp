#pragma once

#include <optional>
#include <string>

namespace porla::Rpc::Methods::Sessions
{
    struct SessionsResumeReq
    {
        int id;
    };

    struct SessionsResumeRes {};
}
