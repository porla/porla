#pragma once

#include <optional>
#include <string>

namespace porla::Methods
{
    struct SessionsPauseReq
    {
        std::optional<std::string> name;
    };

    struct SessionsPauseRes {};
}
