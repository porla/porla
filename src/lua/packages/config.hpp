#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Config
    {
    public:
        static void Register(sol::state& lua);
    };
}
