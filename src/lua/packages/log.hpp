#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Log
    {
    public:
        static void Register(sol::state& lua);
    };
}
