#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Json
    {
    public:
        static void Register(sol::state& lua);
    };
}
