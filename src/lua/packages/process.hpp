#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Process
    {
    public:
        static void Register(sol::state& lua);
    };
}
