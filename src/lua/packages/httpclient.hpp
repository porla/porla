#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    struct HttpClient
    {
        static sol::object Load(sol::this_state s);
    };
}
