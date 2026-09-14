#include "print.hpp"

#include <boost/log/trivial.hpp>

#include <fmt/args.h>
#include <fmt/format.h>

#include "pluginstate.hpp"

void PushArgument(
    fmt::dynamic_format_arg_store<fmt::format_context>& store,
    const sol::stack_proxy& arg);

void porla::Lua::Print(sol::this_state ts, const std::string& pattern, sol::variadic_args args)
{
    fmt::dynamic_format_arg_store<fmt::format_context> store;

    for (const auto& arg : args)
    {
        PushArgument(store, arg);
    }

    sol::state_view lua(ts);

    auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
    auto state = weak.lock();

    std::string msg;

    try
    {
        msg = fmt::vformat(pattern, store);
    }
    catch (const fmt::format_error& e)
    {
        msg = std::string("<bad format: ") + e.what() + ">";
    }

    if (state == nullptr)
    {
        BOOST_LOG_TRIVIAL(info) << "plugin[unknown] " << msg;
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "plugin[" << state->plugin_id << "] " << msg;
    }
}


void PushArgument(
    fmt::dynamic_format_arg_store<fmt::format_context>& store,
    const sol::stack_proxy& arg)
{
    switch (arg.get_type())
    {
    case sol::type::number:
        if (lua_isinteger(arg.lua_state(), arg.stack_index()))
            store.push_back(arg.as<std::int64_t>());
        else
            store.push_back(arg.as<double>());
        break;

    case sol::type::boolean:
        store.push_back(arg.as<bool>());
        break;

    case sol::type::string:
        store.push_back(arg.as<std::string>());
        break;

    default:
    {
        lua_State* L = arg.lua_state();
        luaL_tolstring(L, arg.stack_index(), nullptr);
        store.push_back(std::string(lua_tostring(L, -1)));
        lua_pop(L, 1);
        break;
    }
    }
}