#pragma once

#include <map>
#include <string>
#include <vector>

namespace porla
{
    class Zip
    {
    public:
        static std::map<std::string, std::vector<char>> Load(const std::vector<char>& buffer);
    };
}
