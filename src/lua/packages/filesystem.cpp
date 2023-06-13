#include "filesystem.hpp"

#include <filesystem>

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

        return fs;
    };
}
