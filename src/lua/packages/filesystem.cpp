#include "filesystem.hpp"

#include <filesystem>

namespace fs = std::filesystem;

using porla::Lua::Packages::Filesystem;

sol::object Filesystem::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("create_directories", [](const std::string& path) -> std::tuple<std::optional<bool>, std::optional<std::string>>
    {
        std::error_code ec;
        const auto result = fs::create_directories(path, ec);

        if (ec)
        {
            return std::make_tuple(std::nullopt, ec.message());
        }

        return std::make_tuple(result, std::nullopt);
    });

    tbl.set_function("exists", [](const std::string& path) -> std::tuple<std::optional<bool>, std::optional<std::string>>
    {
        std::error_code ec;
        const auto exists = fs::exists(path, ec);

        if (ec)
        {
            return std::make_tuple(std::nullopt, ec.message());
        }

        return std::make_tuple(exists, std::nullopt);
    });

    return tbl;
}
