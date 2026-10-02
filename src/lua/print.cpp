#include "print.hpp"

#include <boost/log/trivial.hpp>

#include "pluginstate.hpp"

void porla::Lua::Print(sol::this_state ts, sol::variadic_args args)
{
    sol::state_view lua(ts);

    auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
    auto state = weak.lock();

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
        BOOST_LOG_TRIVIAL(info) << "plugin[unknown] " << line;
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "plugin[" << state->plugin_id << "] " << line;
    }
}
