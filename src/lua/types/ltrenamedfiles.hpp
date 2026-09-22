#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    struct LtRenamedFiles
    {
        static void Register(sol::state& lua);
    };
}
