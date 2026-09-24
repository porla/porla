#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    struct LtFilenames
    {
        static void Register(sol::state& lua);
    };
}
