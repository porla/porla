#include "sqlite3statement.hpp"

#include <boost/log/trivial.hpp>

using porla::Lua::UserTypes::Sqlite3Statement;

void Sqlite3Statement::Register(sol::state &lua)
{
    sol::usertype<Sqlite3Statement> type = lua.new_usertype<Sqlite3Statement>(
        "sqlite3.Statement",
        sol::no_constructor);

    type["step"] = [](const Sqlite3Statement& self, const sol::function& func)
    {

    };
}

Sqlite3Statement::Sqlite3Statement(sqlite3_stmt *stmt)
    : m_stmt(stmt)
{
}

Sqlite3Statement::~Sqlite3Statement()
{
    sqlite3_finalize(m_stmt);
}
