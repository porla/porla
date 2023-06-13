#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Torrents
    {
    public:
        static void Register(sol::state& lua);
    };
}
