#pragma once

#include <string>
#include <vector>

namespace porla::Utils
{
    class String
    {
    public:
        static std::size_t Levenshtein(std::string_view a, std::string_view b);
        static std::vector<std::string> Split(const std::string& val, const std::string& delim);
        static std::string ToLower(std::string_view input);
    };
}
