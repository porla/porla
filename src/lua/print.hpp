#pragma once

#include <sol/sol.hpp>

namespace porla::Lua
{
    void Print(sol::this_state ts, sol::variadic_args args);
}