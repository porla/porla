#include "filesystem.hpp"

#include <filesystem>

#include <boost/log/trivial.hpp>

namespace fs = std::filesystem;
using porla::Lua::Packages::FileSystem;

void FileSystem::Register(sol::state& lua)
{
    lua["package"]["preload"]["fs"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table fs = lua.create_table();

        fs["abs"] = [](const std::string& path)
        {
            return fs::absolute(path).string();
        };

        fs["dir"] = [](const std::string& path)
        {
            std::vector<std::string> paths;
            for (const auto& e : fs::directory_iterator(path)) paths.emplace_back(e.path());
            return paths;
        };

        fs["exists"] = [](const std::string& path)
        {
            return fs::exists(path);
        };

        fs["ext"] = [](const std::string& path)
        {
            return fs::path(path).extension().string();
        };

        fs["space"] = [](sol::this_state s, const std::string& path) -> sol::reference
        {
            std::error_code ec;
            const auto space_info = fs::space(path, ec);

            if (ec)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to get space information: " << ec.message();
                return sol::nil;
            }

            sol::state_view lua{s};

            sol::table space = lua.create_table();
            space["available"] = space_info.available;
            space["capacity"] = space_info.capacity;
            space["free"] = space_info.free;
            return space;
        };

        return fs;
    };
}
