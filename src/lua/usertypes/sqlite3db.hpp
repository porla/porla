#pragma once

#include <sol/sol.hpp>
#include <sqlite3.h>

namespace porla::Lua::UserTypes
{
    class Sqlite3Db
    {
    public:
        static void Register(sol::state& lua);

        explicit Sqlite3Db(sqlite3* db);
        ~Sqlite3Db();

    private:
        sqlite3* m_db;
    };
}
