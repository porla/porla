#pragma once

#include <toml++/toml.h>

namespace porla
{
    class Config
    {
    public:
        std::optional<int>   supervised_interval;
        std::optional<pid_t> supervised_pid;
        toml::table          tbl;

        static Config Load(int argc, char* argv[]);
    };
}
