#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class PQL
    {
    public:
        static void Register(sol::state& lua);
    };
}
