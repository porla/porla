#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class HttpClient
    {
    public:
        static void Register(sol::state& lua);
    };
}
