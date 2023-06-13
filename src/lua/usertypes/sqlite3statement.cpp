#include "sqlite3statement.hpp"

#include <boost/log/trivial.hpp>

using porla::Lua::UserTypes::Sqlite3Statement;

class Row
{
public:
    explicit Row(sqlite3_stmt* s) : stmt(s) {}
    sqlite3_stmt* stmt;
};

void Sqlite3Statement::Register(sol::state &lua)
{
    sol::usertype<Sqlite3Statement> type = lua.new_usertype<Sqlite3Statement>(
        "sqlite3.Statement",
        sol::no_constructor);

    sol::usertype<Row> row_type = lua.new_usertype<Row>(
        "sqlite3.Statement-Row",
        sol::no_constructor);

    type["bind"] = [](std::shared_ptr<Sqlite3Statement> self, int position, const sol::object& value)
    {
        if (value.is<int>())
        {
            sqlite3_bind_int(self->m_stmt, position, value.as<int>());
        }
        else if (value.is<std::string>())
        {
            const std::string val = value.as<std::string>();
            sqlite3_bind_text(self->m_stmt, position, val.data(), static_cast<int>(val.size()), SQLITE_TRANSIENT);
        }

        return self;
    };

    type["exec"] = [](const Sqlite3Statement& self, const sol::function& func)
    {
        int res = sqlite3_step(self.m_stmt);

        if (res == SQLITE_DONE)
        {
            return;
        }

        BOOST_LOG_TRIVIAL(error) << "Unexpected SQLite return code for exec. If results are expected, use step instead. Expected 101, got " << res;

        throw std::runtime_error("Unexpected SQLite return code for exec: " + std::to_string(res));
    };

    type["step"] = [](const Sqlite3Statement& self, const sol::function& func)
    {
        do
        {
            switch (int res = sqlite3_step(self.m_stmt))
            {
                case SQLITE_DONE:
                    return;
                case SQLITE_ROW:
                {
                    func(std::make_unique<Row>(self.m_stmt));
                    break;
                }
                default:
                    BOOST_LOG_TRIVIAL(error) << "Unexpected SQLite return code " << res;
                    throw std::runtime_error("Unexpected SQLite return code");
            }
        } while (true);
    };

    row_type["int"] = [](const Row& self, const int index)
    {
        return sqlite3_column_int(self.stmt, index);
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
