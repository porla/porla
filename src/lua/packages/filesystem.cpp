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

    tbl.set_function("link", [](sol::this_state ts, const std::string& src, const std::string& dst) -> std::tuple<sol::object, sol::object>
    {
        std::error_code ec;

        // a missing src fails before dst's parents are created, so nothing is left behind
        if (!fs::exists(src, ec) && !ec)
        {
            ec = std::make_error_code(std::errc::no_such_file_or_directory);
        }

        if (const auto parent = fs::path(dst).parent_path(); !ec && !parent.empty())
        {
            fs::create_directories(parent, ec);
        }

        if (!ec)
        {
            fs::create_hard_link(src, dst, ec);
        }

        if (ec)
        {
            return { sol::lua_nil, PoError::Make(ts, porla::Errors::Code::io_error, ec) };
        }

        return { sol::make_object(ts, true), sol::lua_nil };
    });

    tbl.set_function("space", [](sol::this_state ts, const std::string& path) -> std::tuple<sol::object, sol::object>
    {
        std::error_code ec;
        const auto info = fs::space(path, ec);

        if (ec)
        {
            return { sol::lua_nil, PoError::Make(ts, porla::Errors::Code::io_error, ec) };
        }

        sol::table space = sol::state_view(ts).create_table();
        space["available"] = info.available;
        space["capacity"]  = info.capacity;
        space["free"]      = info.free;

        return { space, sol::lua_nil };
    });

    return tbl;
}
