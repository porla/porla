#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class FileSystem
    {
    public:
        static void Register(sol::state& lua);
    };
}
