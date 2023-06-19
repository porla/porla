#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Sqlite
    {
    public:
        static void Register(sol::state& lua);
    };
}
