#include "sqlite.hpp"

#include <boost/log/trivial.hpp>
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

    auto sqliteStatement = lua.new_usertype<SqliteStatement>(
        "sqlite.Statement",
        sol::no_constructor);

    sqliteDatabase["exec"] = [](const SqliteDatabase& self, const std::string& sql)
    {
        sqlite3_exec(self.db, sql.c_str(), nullptr, nullptr, nullptr);
    };

    sqliteDatabase["prepare"] = [](const SqliteDatabase& self, const std::string& sql)
    {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(self.db, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
        return std::make_shared<SqliteStatement>(stmt);
    };

    sqliteStatement["bind"] = [](const std::shared_ptr<SqliteStatement>& self, int pos, const sol::object& value)
    {
        switch (value.get_type())
        {
            case sol::type::lua_nil:
                sqlite3_bind_null(self->stmt, pos);
                break;
            case sol::type::string:
            {
                const std::string val = value.as<std::string>();
                sqlite3_bind_text(self->stmt, pos, val.c_str(), static_cast<int>(val.size()), SQLITE_TRANSIENT);
                break;
            }
            case sol::type::number: // TODO: Is double correct?
                sqlite3_bind_double(self->stmt, pos, value.as<double>());
                break;
            default:
                BOOST_LOG_TRIVIAL(warning) << "Unhandled value type: " << static_cast<int>(value.get_type());
                break;
        }
        return self;
    };

    sqliteStatement["step"] = [](sol::this_state s, const std::shared_ptr<SqliteStatement>& self, const sol::function& callback)
    {
        sol::state_view lua{s};

        do
        {
            switch (int res = sqlite3_step(self->stmt))
            {
                case SQLITE_DONE:
                    return;
                case SQLITE_ROW:
                {
                    sol::table row = lua.create_table();
                    int count = sqlite3_column_count(self->stmt);

                    for (int i = 0; i < count; i++)
                    {
                        int row_index = i + 1;
                        const auto row_name = sqlite3_column_name(self->stmt, i);

                        switch (sqlite3_column_type(self->stmt, i))
                        {
                        case SQLITE_INTEGER:
                        {
                            int row_value  = sqlite3_column_int(self->stmt, i);
                            row[row_index] = row_value;
                            row[row_name]  = row_value;
                            break;
                        }
                        case SQLITE_TEXT:
                        {
                            const auto row_value = sqlite3_column_text(self->stmt, i);
                            row[row_index]       = reinterpret_cast<const char*>(row_value);
                            row[row_name]        = reinterpret_cast<const char*>(row_value);;
                            break;
                        }
                        }
                    }

                    callback(row);

                    break;
                }
                default:
                    BOOST_LOG_TRIVIAL(error) << "Unexpected SQLite return code " << res;
                    throw std::runtime_error("Unexpected SQLite return code");
            }
        } while (true);
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
