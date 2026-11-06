#include "../packages.hpp"

#include "../print.hpp"

using porla::Lua::Packages::Log;

sol::object Log::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("debug",   [](sol::this_state ts, sol::variadic_args args) { porla::Lua::LogArgs(ts, porla::Log::debug, args); });
    tbl.set_function("error",   [](sol::this_state ts, sol::variadic_args args) { porla::Lua::LogArgs(ts, porla::Log::error, args); });
    tbl.set_function("info",    [](sol::this_state ts, sol::variadic_args args) { porla::Lua::LogArgs(ts, porla::Log::info, args); });
    tbl.set_function("warning", [](sol::this_state ts, sol::variadic_args args) { porla::Lua::LogArgs(ts, porla::Log::warning, args); });

    return tbl;
}
