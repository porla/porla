#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    struct Runtime
    {
        static sol::object Load(sol::this_state s);
    };
}
