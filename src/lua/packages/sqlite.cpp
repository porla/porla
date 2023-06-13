#include "sqlite.hpp"

#include <sqlite3.h>

using porla::Lua::Packages::Sqlite;

struct SqliteDatabase
{
    sqlite3* db;

    explicit SqliteDatabase(sqlite3* d) : db(d) {}

    ~SqliteDatabase()
    {
        sqlite3_close(db);
    }
};

struct SqliteStatement
{
    sqlite3_stmt* stmt;

    explicit SqliteStatement(sqlite3_stmt* s) : stmt(s) {}

    ~SqliteStatement()
    {
        sqlite3_finalize(stmt);
    }
};

void Sqlite::Register(sol::state& lua)
{
    auto sqliteDatabase = lua.new_usertype<SqliteDatabase>(
        "sqlite.Database",
        sol::no_constructor);

    sqliteDatabase["exec"] = [](const SqliteDatabase& self, const std::string& sql)
    {
        sqlite3_exec(self.db, sql.c_str(), nullptr, nullptr, nullptr);
    };

    sqliteDatabase["prepare"] = [](const SqliteDatabase& self, const std::string& sql)
    {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(self.db, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
    };

    lua["package"]["preload"]["sqlite"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table sql = lua.create_table();

        sql["open"] = [](const std::string& args)
        {
            sqlite3* db;
            sqlite3_open(args.c_str(), &db);
            return std::make_shared<SqliteDatabase>(db);
        };

        return sql;
    };
}
