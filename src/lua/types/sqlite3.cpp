#include "../types.hpp"

#include <map>
#include <string>

#include <sqlite3.h>

using porla::Lua::Types::Sqlite3;

struct SqliteBlob
{
    std::string bytes;
};

struct SqliteStatement
{
    explicit SqliteStatement(sqlite3_stmt* stmt)
        : m_stmt(stmt)
    {
    }

    void Execute(sol::optional<sol::table> values)
    {
        if (values)
        {
            for (const auto& [k, v] : *values)
            {
                if (k.get_type() != sol::type::string)
                {
                    throw sol::error("bind key must be a string or integer");
                }

                const std::string name = k.as<std::string>();
                const int idx = sqlite3_bind_parameter_index(m_stmt, name.c_str());

                if (idx == 0)
                {
                    throw sol::error("unknown bind parameter: " + name);
                }

                int rc = SQLITE_OK;

                switch (v.get_type())
                {
                    case sol::type::lua_nil:
                        rc = sqlite3_bind_null(m_stmt, idx);
                        break;

                    case sol::type::boolean:
                        rc = sqlite3_bind_int(m_stmt, idx, v.as<bool>() ? 1 : 0);
                        break;

                    case sol::type::number:
                    {
                        lua_State* L = v.lua_state();
                        sol::stack::push(L, v);
                        const bool is_int = lua_isinteger(L, -1);
                        lua_pop(L, 1);

                        if (is_int)
                        {
                            rc = sqlite3_bind_int64(m_stmt, idx, v.as<sqlite3_int64>());
                        }
                        else
                        {
                            rc = sqlite3_bind_double(m_stmt, idx, v.as<double>());
                        }
                        break;
                    }

                    case sol::type::string:
                    {
                        const std::string val = v.as<std::string>();

                        rc = sqlite3_bind_text(
                            m_stmt,
                            idx,
                            val.data(),
                            static_cast<int>(val.size()),
                            SQLITE_TRANSIENT);

                        break;
                    }

                    case sol::type::userdata:
                    {
                        if (v.is<SqliteBlob>())
                        {
                            const auto& b = v.as<SqliteBlob>();

                            rc = sqlite3_bind_blob(
                                m_stmt,
                                idx,
                                b.bytes.data(),
                                static_cast<int>(b.bytes.size()),
                                SQLITE_TRANSIENT);

                            break;
                        }

                        throw sol::error("unsupported userdata bind value");
                    }

                    default:
                        throw sol::error("unsupported bind value type");
                }

                if (rc != SQLITE_OK)
                {
                    throw sol::error("bind failed");
                }
            }
        }

        const int rc = sqlite3_step(m_stmt);

        if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        {
            throw sol::error("execute failed");
        }

        sqlite3_reset(m_stmt);
        sqlite3_clear_bindings(m_stmt);
    }

    void Finalize()
    {
        const auto rc = sqlite3_finalize(m_stmt);

        if (rc != SQLITE_OK)
        {
            throw sol::error("failed to finalize statement");
        }
    }

    std::function<sol::object(sol::this_state)> RowsIterator()
    {
        return [&](sol::this_state ts) -> sol::object
        {
            switch (sqlite3_step(m_stmt))
            {
            case SQLITE_DONE:
                return sol::make_object(ts, sol::nil);

            case SQLITE_ROW:
                return sol::make_object(ts, GetRow(ts));
            }

            throw sol::error("unknown status on step");
        };
    }

private:
    sol::object GetColumn(sol::this_state s, int col) const
    {
        switch (sqlite3_column_type(m_stmt, col))
        {
            case SQLITE_INTEGER:
                return sol::make_object(s, sqlite3_column_int64(m_stmt, col));
            case SQLITE_FLOAT:
                return sol::make_object(s, sqlite3_column_double(m_stmt, col));
            case SQLITE_TEXT:
            {
                const auto n = sqlite3_column_bytes(m_stmt, col);
                const auto* p = sqlite3_column_text(m_stmt, col);

                return sol::make_object(s,
                    std::string(
                        reinterpret_cast<const char*>(p),
                        static_cast<std::size_t>(n)));
            }
            case SQLITE_BLOB:
            {
                const auto n = sqlite3_column_bytes(m_stmt, col);
                const auto* p = sqlite3_column_blob(m_stmt, col);
                return sol::make_object(s,
                    std::string(
                        reinterpret_cast<const char*>(p),
                        static_cast<std::size_t>(n)));
            }
            case SQLITE_NULL:
            default:
                return sol::make_object(s, sol::nil);
        }
    }

    sol::table GetRow(sol::this_state s) const
    {
        sol::state_view lua{s};
        sol::table row = lua.create_table();

        for (int i = 0; i < sqlite3_column_count(m_stmt); i++)
        {
            const char* name = sqlite3_column_name(m_stmt, i);
            row[name ? name : std::to_string(i)] = GetColumn(s, i);
        }
        return row;
    }

    sqlite3_stmt* m_stmt;
};

struct SqliteDatabase
{
    explicit SqliteDatabase(sqlite3* db)
        : m_db(db)
    {
    }

    void Close()
    {
        const auto rc = sqlite3_close(m_db);

        if (rc != SQLITE_OK)
        {
            throw sol::error("failed to close db");
        }
    }

    void Exec(const std::string& sql)
    {
        const auto rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr);

        if (rc != SQLITE_OK)
        {
            throw sol::error("failed to execute statement");
        }
    }

    std::shared_ptr<SqliteStatement> Prepare(const std::string& sql)
    {
        sqlite3_stmt* stmt = nullptr;
        const auto rc = sqlite3_prepare_v2(m_db, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

        if (rc != SQLITE_OK)
        {
            throw sol::error("failed to prepare statement");
        }

        return std::make_shared<SqliteStatement>(stmt);
    }

    sol::object Query(sol::this_state L, const std::string& sql)
    {
        sqlite3_stmt* stmt = nullptr;

        int rc = sqlite3_prepare_v2(
            m_db,
            sql.c_str(),
            static_cast<int>(sql.size()),
            &stmt,
            nullptr);

        std::map<std::string, int> cols;

        sol::state_view lua{L};
        sol::table rows = lua.create_table();

        do
        {
            rc = sqlite3_step(stmt);

            if (rc == SQLITE_DONE)
            {
                break;
            }

            sol::table row = lua.create_table();

            for (int i = 0; i < sqlite3_column_count(stmt); i++)
            {
                row[sqlite3_column_name(stmt, i)] = sqlite3_column_int64(stmt, i);
            }

            rows.add(row);
        } while (rc == SQLITE_ROW);

        sqlite3_finalize(stmt);

        return rows;
    }

private:
    sqlite3* m_db;
};

void Sqlite3::Register(sol::state& lua)
{
    sol::table sqlite = lua["sqlite3"].valid()
        ? lua["sqlite3"].get<sol::table>()
        : lua.create_named_table("sqlite3");

    sqlite.new_usertype<SqliteBlob>(
        "Blob",
        sol::no_constructor,
        "size", sol::property([](const SqliteBlob& b) { return b.bytes.size(); })
    );

    sqlite.new_usertype<SqliteDatabase>(
        "Database",
        sol::no_constructor,
        "close", &SqliteDatabase::Close,
        "exec", &SqliteDatabase::Exec,
        "prepare", &SqliteDatabase::Prepare,
        "query", &SqliteDatabase::Query
    );

    sqlite.new_usertype<SqliteStatement>(
        "Statement",
        sol::no_constructor,
        "execute", &SqliteStatement::Execute,
        "finalize", &SqliteStatement::Finalize,
        "rows", &SqliteStatement::RowsIterator
    );

    sqlite["blob"] = [](const std::string& bytes)
    {
        return SqliteBlob{bytes};
    };

    sqlite["open"] = [](const std::string& filename)
    {
        sqlite3* db = nullptr;
        const auto rc = sqlite3_open(filename.c_str(), &db);

        if (rc != SQLITE_OK)
        {
            std::string err = db ? sqlite3_errmsg(db) : "unable to open database";

            if (db)
            {
                sqlite3_close(db);
            }

            throw sol::error(err);
        }

        return std::make_shared<SqliteDatabase>(db);
    };

    sqlite["DONE"] = SQLITE_DONE;
    sqlite["ERROR"] = SQLITE_ERROR;
    sqlite["OK"] = SQLITE_OK;
    sqlite["ROW"] = SQLITE_ROW;
}
