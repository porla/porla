#pragma once

#include <string>
#include <vector>

namespace porla::Utils
{
    class String
    {
    public:
        static std::vector<std::string> Split(const std::string& val, const std::string& delim);
    };
}
