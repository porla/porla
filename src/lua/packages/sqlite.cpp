#include "../packages.hpp"

#include <boost/log/trivial.hpp>
#include <sqlite3.h>

using porla::Lua::Packages::Sqlite;

struct SqliteDatabase
{
    sqlite3* db;

    explicit SqliteDatabase(sqlite3* d) : db(d) {}

    int Close()
    {
        if (db != nullptr)
        {
            int res = sqlite3_close(db);
            db = nullptr;
            return res;
        }

        return SQLITE_ERROR;
    }

    ~SqliteDatabase()
    {
        if (db != nullptr)
        {
            sqlite3_close(db);
        }
    }
};

struct SqliteStatement
{
    sqlite3_stmt* stmt;

    explicit SqliteStatement(sqlite3_stmt* s) : stmt(s) {}

    ~SqliteStatement()
    {
        if (stmt != nullptr)
        {
            sqlite3_finalize(stmt);
        }
    }

    int Finalize()
    {
        if (stmt == nullptr)
        {
            return SQLITE_ERROR;
        }

        int res = sqlite3_finalize(stmt);
        stmt = nullptr;
        return res;
    }

    [[nodiscard]] sol::object GetValue(sol::this_state s, int position) const
    {
        switch (sqlite3_column_type(stmt, position))
        {
            case SQLITE_INTEGER:
                return sol::make_object(s, sqlite3_column_int64(stmt, position));

            case SQLITE_FLOAT:
                return sol::make_object(s, sqlite3_column_double(stmt, position));

            case SQLITE_TEXT:
                return sol::make_object(
                    s,
                    std::string(
                        reinterpret_cast<const char*>(sqlite3_column_text(stmt, position),
                        sqlite3_column_bytes(stmt, position))));

            case SQLITE_NULL:
            default:
                return sol::nil;
        }
    }
};

void Sqlite::Register(sol::state& lua)
{
    auto sqliteDatabase = lua.new_usertype<SqliteDatabase>(
        "sqlite3.Database",
        sol::no_constructor);

    auto sqliteStatement = lua.new_usertype<SqliteStatement>(
        "sqlite3.Statement",
        sol::no_constructor);

    sqliteDatabase["close"] = &SqliteDatabase::Close;

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

    sqliteStatement["exec"] = [](const std::shared_ptr<SqliteStatement>& self)
    {
        sqlite3_step(self->stmt);
    };

    sqliteStatement["finalize"] = &SqliteStatement::Finalize;
    sqliteStatement["get_value"] = &SqliteStatement::GetValue;

    sqliteStatement["step"] = [](sol::this_state s, const std::shared_ptr<SqliteStatement>& self, const sol::function& callback)
    {
        sol::state_view lua{s};
        return sqlite3_step(self->stmt);
    };

    lua["package"]["preload"]["sqlite3"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table sql = lua.create_table();

        sql["open"] = [](const std::string& args)
        {
            sqlite3* db;
            sqlite3_open(args.c_str(), &db);
            return std::make_shared<SqliteDatabase>(db);
        };

        sql["DONE"] = SQLITE_DONE;
        sql["ERROR"] = SQLITE_ERROR;
        sql["OK"] = SQLITE_OK;
        sql["ROW"] = SQLITE_ROW;

        return sql;
    };
}
