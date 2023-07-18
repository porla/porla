#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Workflows
    {
    public:
        static void Register(sol::state& lua);
    };
}
