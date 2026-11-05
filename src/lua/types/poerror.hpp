#pragma once

#include <stdexcept>
#include <string>
#include <system_error>

#include <libtorrent/error_code.hpp>
#include <sol/sol.hpp>

#include "../../errors.hpp"

namespace porla::Lua
{
    struct Error : std::runtime_error
    {
        Error(Errors::Code code, const std::string& message)
            : std::runtime_error(message)
            , code(code)
        {
        }

        Errors::Code code;
    };

    std::string DescribeError(const sol::protected_function_result& result);
}

namespace porla::Lua::Types
{
    class PoError
    {
    public:
        static void Register(sol::state& lua);

        static sol::table Make(lua_State* L, Errors::Code code, const std::string& message);
        static sol::table Make(lua_State* L, Errors::Code code, const lt::error_code& ec, const std::string& message = {});
        static sol::table Make(lua_State* L, Errors::Code code, const std::error_code& ec, const std::string& message = {});
        static sol::object Construct(sol::this_state ts, const lt::error_code& ec, Errors::Code code = Errors::Code::libtorrent_error);
    };
}
