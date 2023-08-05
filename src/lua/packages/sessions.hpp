#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Sessions
    {
    public:
        static void Register(sol::state& lua);
    };
}
