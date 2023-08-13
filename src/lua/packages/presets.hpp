#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Presets
    {
    public:
        static void Register(sol::state& lua);
    };
}
