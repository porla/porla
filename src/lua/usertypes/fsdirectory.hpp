#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::UserTypes
{
    class FsDirectoryEntry;

    class FsDirectory
    {
    public:
        static void Register(sol::state& lua);

        explicit FsDirectory(sol::object args);

    private:
        std::vector<FsDirectoryEntry> Iterate();
        sol::object m_args;
    };
}
