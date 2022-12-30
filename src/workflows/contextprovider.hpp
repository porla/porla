#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace porla::Workflows
{
    class ContextProvider
    {
    public:
        virtual nlohmann::json ResolveSegments(const std::vector<std::string>& segments) = 0;
    };
}
