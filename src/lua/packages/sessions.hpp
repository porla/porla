#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    struct Sessions
    {
        static sol::object Load(sol::this_state s);
    };
}
