#pragma once

#include <toml++/toml.h>

namespace porla
{
    class Config
    {
    public:
        static toml::table Load(int argc, char* argv[]);
    };
}
