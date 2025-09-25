#include "statement.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Statement;

class InternalRow : public Statement::IRow
{
public:
    explicit InternalRow(sqlite3_stmt* stmt)
        : m_stmt(stmt)
    {
    }

    [[nodiscard]] int GetInt32(int pos) const override
    {
        return sqlite3_column_int(m_stmt, pos);
    }

    [[nodiscard]] std::optional<int> GetOptionalInt32(int pos) const override
    {
        if (sqlite3_column_type(m_stmt, pos) == SQLITE_NULL) return std::nullopt;
        return sqlite3_column_int(m_stmt, pos);
    }

    [[nodiscard]] std::vector<char> GetBuffer(int pos) const override
    {
        int len = sqlite3_column_bytes(m_stmt, pos);
        const char* buf = static_cast<const char*>(sqlite3_column_blob(m_stmt, pos));
        return {buf, buf + len};
    }

    [[nodiscard]] std::string GetStdString(int pos) const override
    {
        auto data = sqlite3_column_text(m_stmt, pos);
        return
        {
            reinterpret_cast<const char*>(data),
            static_cast<unsigned long>(sqlite3_column_bytes(m_stmt, pos))
        };
    }

private:
    sqlite3_stmt* m_stmt;
};

Statement::Statement(sqlite3_stmt *stmt)
    : m_stmt(stmt)
{
}

Statement::~Statement()
{
    if (sqlite3_finalize(m_stmt) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to finalize SQLite statement";
    }
}

Statement Statement::Prepare(sqlite3 *db, const std::string_view &sql)
{
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.data(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to prepare SQLite statement: " << sqlite3_errmsg(db);
        throw std::runtime_error("Failed to prepare SQLite statement: " + std::string(sqlite3_errmsg(db)));
    }

    return Statement(stmt);
}

Statement& Statement::Bind(int pos, int value)
{
    if (sqlite3_bind_int(m_stmt, pos, value) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to bind SQLite value";
        throw std::runtime_error("Failed to bind SQLite value");
    }

    return *this;
}

Statement& Statement::Bind(int pos, const std::optional<int>& value)
{
    int res = value == std::nullopt
        ? sqlite3_bind_null(m_stmt, pos)
        : sqlite3_bind_int(m_stmt, pos, value.value());

    if (res != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to bind SQLite value";
        throw std::runtime_error("Failed to bind SQLite value");
    }

    return *this;
}

Statement& Statement::Bind(int pos, const std::string_view &value)
{
    if (sqlite3_bind_text(m_stmt, pos, value.data(), static_cast<int>(value.size()), nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to bind SQLite value";
        throw std::runtime_error("Failed to bind SQLite value");
    }

    return *this;
}

Statement& Statement::Bind(int pos, const std::optional<std::string_view> &value)
{
    int res = value == std::nullopt
        ? sqlite3_bind_null(m_stmt, pos)
        : sqlite3_bind_text(m_stmt, pos, value->data(), static_cast<int>(value->size()), nullptr);

    if (res != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to bind SQLite value";
        throw std::runtime_error("Failed to bind SQLite value");
    }

    return *this;
}

Statement& Statement::Bind(int pos, const std::vector<char>& buffer)
{
    if (sqlite3_bind_blob(m_stmt, pos, buffer.data(), static_cast<int>(buffer.size()), SQLITE_TRANSIENT) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to bind SQLite value";
        throw std::runtime_error("Failed to bind SQLite value");
    }

    return *this;
}

void Statement::Execute()
{
    int res = sqlite3_step(m_stmt);

    if (res == SQLITE_DONE)
    {
        return;
    }

    BOOST_LOG_TRIVIAL(error) << "Unexpected SQLite return code for Execute. If results are expected, use Step instead. Expected 101, got " << res;

    throw std::runtime_error("Unexpected SQLite return code for Execute: " + std::to_string(res));
}

void Statement::Step(const std::function<int(const Statement::IRow&)>& cb)
{
    do
    {
        switch (int res = sqlite3_step(m_stmt))
        {
        case SQLITE_DONE:
            return;
        case SQLITE_ROW:
        {
            InternalRow r(m_stmt);
            cb(r);
            break;
        }
        default:
            BOOST_LOG_TRIVIAL(error) << "Unexpected SQLite return code " << res;
            throw std::runtime_error("Unexpected SQLite return code");
        }
    } while (true);
}
