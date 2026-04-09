#include "../types.hpp"

#include <filesystem>

namespace fs = std::filesystem;

using porla::Lua::Types::Fs;

template <>
struct sol::is_container<fs::path> : std::false_type {};

void Fs::Register(sol::state& lua)
{
    sol::table fs = lua["fs"].valid()
        ? lua["fs"].get<sol::table>()
        : lua.create_named_table("fs");

    fs["path"] = lua.new_usertype<fs::path>(
        "fs.path",
        sol::call_constructor, sol::factories(
            []() { return fs::path(); },
            [](const std::string& p) { return fs::path(p); }
        ),
        sol::meta_function::division, sol::overload(
            [](const fs::path& a, const std::string& b) { return a / b; },
            [](const fs::path& a, const fs::path& b) { return a / b; }
        ),
        sol::meta_function::to_string, [](const fs::path& p) { return p.string(); }
    );

    fs["current_path"] = []() { return fs::current_path(); };

    fs["exists"] = sol::overload(
        [](const fs::path& path)    { return fs::exists(path); },
        [](const std::string& path) { return fs::exists(path); }
    );

    fs["is_regular_file"] = sol::overload(
        [](const fs::path& path)    { return fs::is_regular_file(path); },
        [](const std::string& path) { return fs::is_regular_file(path); }
    );
}
