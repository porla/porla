#pragma once

#include <sol/sol.hpp>

namespace porla::Lua
{
    void Print(sol::this_state ts, const std::string& pattern, sol::variadic_args args);
}