#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    struct HttpServer
    {
        static sol::object Load(sol::this_state s);
    };
}
