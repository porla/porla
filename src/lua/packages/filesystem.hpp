#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    struct Filesystem
    {
        static sol::object Load(sol::this_state s);
    };
}
