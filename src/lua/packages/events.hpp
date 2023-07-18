#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Events
    {
    public:
        static void Register(sol::state& lua);
    };
}
