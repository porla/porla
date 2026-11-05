#include "../packages.hpp"

#include <filesystem>

#include "../types/poerror.hpp"

namespace fs = std::filesystem;

using porla::Lua::Packages::Filesystem;
using porla::Lua::Types::PoError;

sol::object Filesystem::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("create_directories", [](sol::this_state ts, const std::string& path) -> std::tuple<sol::object, sol::object>
    {
        std::error_code ec;
        const auto result = fs::create_directories(path, ec);

        if (ec)
        {
            return { sol::lua_nil, PoError::Make(ts, porla::Errors::Code::io_error, ec) };
        }

        return { sol::make_object(ts, result), sol::lua_nil };
    });

    tbl.set_function("exists", [](sol::this_state ts, const std::string& path) -> std::tuple<sol::object, sol::object>
    {
        std::error_code ec;
        const auto exists = fs::exists(path, ec);

        if (ec)
        {
            return { sol::lua_nil, PoError::Make(ts, porla::Errors::Code::io_error, ec) };
        }

        return { sol::make_object(ts, exists), sol::lua_nil };
    });

    return tbl;
}
