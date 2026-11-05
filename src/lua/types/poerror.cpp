#include "poerror.hpp"

#include <boost/system/system_error.hpp>

using porla::Lua::Types::PoError;

namespace
{
    constexpr const char* kMetatable = "porla.PoError";

    std::string ToString(lua_State* L, int index)
    {
        std::size_t len  = 0;
        const char* text = luaL_tolstring(L, index, &len);
        std::string out(text, len);

        lua_pop(L, 1);

        return out;
    }

    // sol2's default handler replaces any non-string error with "An unknown error has triggered the
    // default error handler", which would erase a PoError before the host can log it.
    int ErrorHandler(lua_State* L)
    {
        if (lua_type(L, 1) == LUA_TTABLE)
        {
            lua_settop(L, 1);
            return 1;
        }

        return sol::default_traceback_error_handler(L);
    }
}

void PoError::Register(sol::state& lua)
{
    sol::table mt = lua.create_table();

    mt["__name"]     = "PoError";
    mt["__tostring"] = [](const sol::table& err) -> std::string
    {
        return err.get_or<std::string>("code", "error") + ": " + err.get_or<std::string>("message", "");
    };
    mt["__concat"]   = [](sol::this_state ts, const sol::stack_object& a, const sol::stack_object& b)
    {
        return ToString(ts, a.stack_index()) + ToString(ts, b.stack_index());
    };

    lua.registry()[kMetatable] = mt;

    // protected functions copy the default handler when they are created, so this must run first
    sol::protected_function::set_default_handler(
        sol::object(lua.lua_state(), sol::in_place, sol::c_call<decltype(&ErrorHandler), &ErrorHandler>));

    lua.set_exception_handler([](lua_State* L, sol::optional<const std::exception&> ex, sol::string_view description) -> int
    {
        if (const auto* err = ex ? dynamic_cast<const porla::Lua::Error*>(&*ex) : nullptr)
        {
            return sol::stack::push(L, PoError::Make(L, err->code, err->what()));
        }

        if (const auto* err = ex ? dynamic_cast<const boost::system::system_error*>(&*ex) : nullptr)
        {
            const auto& ec = err->code();

            const auto code = ec == lt::errors::invalid_torrent_handle  ? Errors::Code::torrent_not_found
                            : ec.category() == lt::libtorrent_category() ? Errors::Code::libtorrent_error
                            : Errors::Code::io_error;

            return sol::stack::push(L, PoError::Construct(L, ec, code));
        }

        return sol::stack::push(L, description);
    });
}

sol::table PoError::Make(lua_State* L, Errors::Code code, const std::string& message)
{
    sol::state_view lua(L);

    sol::table err = lua.create_table();
    err["code"]    = std::string(Errors::Get(code).name);
    err["message"] = message;
    err[sol::metatable_key] = lua.registry()[kMetatable];

    return err;
}

sol::object PoError::Construct(sol::this_state ts, const lt::error_code& ec, Errors::Code code)
{
    if (!ec)
    {
        return sol::lua_nil;
    }

    sol::table err = Make(ts, code, ec.message());
    err["category"] = ec.category().name();
    err["value"]    = ec.value();

    return err;
}

std::string porla::Lua::DescribeError(const sol::protected_function_result& result)
{
    if (result.valid() || result.return_count() < 1)
    {
        return "unknown error";
    }

    // luaL_tolstring honours __tostring, so a PoError reads "code: message" instead of an empty string.
    // result.get<> refuses to read a failed result, so take the error value off the stack directly.
    return ToString(result.lua_state(), result.stack_index());
}
