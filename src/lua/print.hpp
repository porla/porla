#pragma once

#include <sol/sol.hpp>

#include "../logger.hpp"

namespace porla::Lua
{
    void LogArgs(sol::this_state ts, porla::Log::Level level, sol::variadic_args args);
    void Print(sol::this_state ts, sol::variadic_args args);
}
