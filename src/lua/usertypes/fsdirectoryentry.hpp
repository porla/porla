#pragma once

#include <filesystem>
#include <sol/sol.hpp>

namespace porla::Lua::UserTypes
{
    class FsDirectoryEntry
    {
    public:
        static void Register(sol::state& lua);

        explicit FsDirectoryEntry(std::filesystem::directory_entry  p);

    private:
        std::filesystem::directory_entry m_entry;
    };
}
