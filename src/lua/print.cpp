#include "print.hpp"

#include "pluginstate.hpp"

void porla::Lua::LogArgs(sol::this_state ts, porla::Log::Level level, sol::variadic_args args)
{
    auto state = LuaState::From(ts);

    std::string line;

    for (const auto& arg : args)
    {
        std::size_t len = 0;

        // Pushes the string representation; indices in `args` are absolute,
        // so they survive the push.
        const char* str = luaL_tolstring(ts, arg.stack_index(), &len);

        if (!line.empty()) line += '\t';
        line.append(str, len);

        lua_pop(ts, 1);
    }

    if (state == nullptr)
    {
        static auto unknown = porla::Log::Component("plugin");
        BOOST_LOG_SEV(unknown, level) << line;
    }
    else
    {
        BOOST_LOG_SEV(state->logger, level) << line;
    }
}

void porla::Lua::Print(sol::this_state ts, sol::variadic_args args)
{
    LogArgs(ts, porla::Log::info, args);
}
