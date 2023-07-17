#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Timers
    {
    public:
        static void Register(sol::state& lua);
    };
}
