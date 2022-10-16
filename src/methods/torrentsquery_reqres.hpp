#pragma once

#include <string>

namespace porla::Methods
{
    struct TorrentsQueryReq
    {
        std::string query;
    };

    struct TorrentsQueryRes
    {
        std::vector<nlohmann::json> results;
    };
}
