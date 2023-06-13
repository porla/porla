#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Cron
    {
    public:
        static void Register(sol::state& lua);
    };
}
