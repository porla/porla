#include "../types.hpp"

#include <boost/log/trivial.hpp>

using porla::Lua::Types::Log;

void Log::Register(sol::state& lua)
{
    sol::table log = lua["log"].valid()
        ? lua["log"].get<sol::table>()
        : lua.create_named_table("log");

    log["info"] = [](const sol::variadic_args& args)
    {
        BOOST_LOG_TRIVIAL(info) << args[0].get<std::string>();
    };

    log["warning"] = [](const sol::variadic_args& args)
    {
        BOOST_LOG_TRIVIAL(warning) << args[0].get<std::string>();
    };

    log["error"] = [](const sol::variadic_args& args)
    {
        BOOST_LOG_TRIVIAL(error) << args[0].get<std::string>();
    };
}
