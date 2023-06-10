#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::UserTypes
{
    class FsDirectory
    {
    public:
        static void Register(sol::state& lua);

        explicit FsDirectory(sol::object args);

    private:
        std::vector<std::string> Iterate();
        sol::object m_args;
    };
}
