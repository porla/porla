#pragma once

#include <string>

namespace porla
{
    struct Uri
    {
        static bool Parse(const std::string& value, Uri& out);

        std::string   scheme;
        std::string   host;
        std::uint16_t port;
        std::string   path;
    };
}