#pragma once

#include <string>

namespace porla::Utils
{
    class SecretKey
    {
    public:
        static std::string New(int length = 32);
    };
}