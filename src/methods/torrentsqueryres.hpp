#pragma once

#include <vector>

#include <nlohmann/json.hpp>

namespace porla::Methods
{
    struct TorrentsQueryRes
    {
        std::vector<nlohmann::json> results;
    };
}
