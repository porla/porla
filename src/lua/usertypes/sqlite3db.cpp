#include "sqlite3db.hpp"

#include <boost/log/trivial.hpp>

#include "sqlite3statement.hpp"

using porla::Lua::UserTypes::Sqlite3Db;

void Sqlite3Db::Register(sol::state &lua)
{
    sol::usertype<Sqlite3Db> type = lua.new_usertype<Sqlite3Db>(
        "sqlite3.Database",
        sol::no_constructor);

    type["exec"] = [](const Sqlite3Db& self, const std::string& sql)
    {
        sqlite3_exec(self.m_db, sql.c_str(), nullptr, nullptr, nullptr);
    };

    type["prepare"] = [](const Sqlite3Db& self, const std::string& sql)
    {
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(self.m_db, sql.data(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to prepare SQLite statement: " << sqlite3_errmsg(self.m_db);
            throw std::runtime_error("Failed to prepare SQLite statement: " + std::string(sqlite3_errmsg(self.m_db)));
        }

        return std::make_shared<Sqlite3Statement>(stmt);
    };
}

Sqlite3Db::Sqlite3Db(sqlite3 *db)
    : m_db(db)
{
}

Sqlite3Db::~Sqlite3Db()
{
    sqlite3_close(m_db);
}
