#pragma once

#include <nlohmann/json.hpp>

namespace porla::Workflows
{
    struct Step
    {
        std::string    uses;
        nlohmann::json with;
    };
}
