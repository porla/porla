#pragma once

#include <optional>
#include <string>

namespace porla::Methods
{
    struct TorrentsAddReq
    {
        std::optional<std::string> magnet_uri;
        std::string save_path;
        std::optional<std::string> ti;
    };
}
