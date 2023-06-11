#pragma once

#include <sol/sol.hpp>
#include <sqlite3.h>

namespace porla::Lua::UserTypes
{
    class Sqlite3Statement
    {
    public:
        static void Register(sol::state& lua);

        explicit Sqlite3Statement(sqlite3_stmt* stmt);
        ~Sqlite3Statement();

    private:
        sqlite3_stmt* m_stmt;
    };
}
