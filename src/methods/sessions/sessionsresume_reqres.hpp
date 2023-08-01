#pragma once

#include <optional>
#include <string>

namespace porla::Methods
{
    struct SessionsResumeReq
    {
        std::optional<std::string> name;
    };

    struct SessionsResumeRes {};
}
