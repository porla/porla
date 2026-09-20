#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    struct Presets
    {
        static sol::object Load(sol::this_state s);
    };
}
