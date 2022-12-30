#pragma once

#include <nlohmann/json.hpp>

namespace porla::Workflows
{
    struct Step
    {
        std::string                           uses;
        std::map<std::string, nlohmann::json> with;
    };
}
