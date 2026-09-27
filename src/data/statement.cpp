#include "statement.hpp"

#include <map>

#include <boost/log/trivial.hpp>

using porla::Data::Statement;

class InternalRow : public Statement::IRow
{
public:
    explicit InternalRow(sqlite3_stmt* stmt, const std::map<std::string, int>& cols)
        : m_stmt(stmt)
        , m_cols(cols)
    {
    }

    [[nodiscard]] int GetInt32(const std::string& col) const override
    {
        const auto type = sqlite3_column_type(m_stmt, m_cols.at(col));

        if (type != SQLITE_INTEGER)
        {
            throw std::runtime_error("Invalid column type - expected INTEGER, found: " + std::to_string(type));
        }

        return sqlite3_column_int(m_stmt, m_cols.at(col));
    }

    [[nodiscard]] std::int64_t GetInt64(const std::string& col) const override
    {
        const auto type = sqlite3_column_type(m_stmt, m_cols.at(col));

        if (type != SQLITE_INTEGER)
        {
            throw std::runtime_error("Invalid column type - expected INTEGER, found: " + std::to_string(type));
        }

        return sqlite3_column_int64(m_stmt, m_cols.at(col));
    }

    [[nodiscard]] std::optional<int> GetOptionalInt32(const std::string& col) const override
    {
        const auto type = sqlite3_column_type(m_stmt, m_cols.at(col));

        if (type == SQLITE_NULL)
        {
            return std::nullopt;
        }

        return GetInt32(col);
    }

    [[nodiscard]] std::optional<std::int64_t> GetOptionalInt64(const std::string& col) const override
    {
        const auto type = sqlite3_column_type(m_stmt, m_cols.at(col));

        if (type == SQLITE_NULL)
        {
            return std::nullopt;
        }

        return GetInt64(col);
    }

    [[nodiscard]] std::vector<char> GetBuffer(const std::string& col) const override
    {
        const auto len = sqlite3_column_bytes(m_stmt, m_cols.at(col));
        const auto blob = sqlite3_column_blob(m_stmt, m_cols.at(col));
        const auto buf = static_cast<const char*>(blob);

        return {buf, buf + len};
    }

    [[nodiscard]] std::string GetStdString(const std::string& col) const override
    {
        const auto type = sqlite3_column_type(m_stmt, m_cols.at(col));

        if (type == SQLITE_NULL)
        {
            throw std::runtime_error("Null value found - if null values are expected, use GetOptionalStdString: " + col);
        }

        const auto bytes = sqlite3_column_bytes(m_stmt, m_cols.at(col));

        if (bytes == 0)
        {
            return {};
        }

        const auto* data = sqlite3_column_text(m_stmt, m_cols.at(col));

        if (data == nullptr)
        {
            return {};
        }

        return std::string(
            reinterpret_cast<const char*>(data), 
            static_cast<std::size_t>(bytes));
    }

    [[nodiscard]] std::optional<std::string> GetOptionalStdString(const std::string& col) const override
    {
        const auto type = sqlite3_column_type(m_stmt, m_cols.at(col));

        if (type == SQLITE_NULL)
        {
            return std::nullopt;
        }

        return GetStdString(col);
    }

private:
    sqlite3_stmt* m_stmt;
    const std::map<std::string, int>& m_cols;
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
Statement& Statement::Check(int res, const std::string& param)
{
    if (res != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind '" + param + "': " + sqlite3_errstr(res)
            + " (" + sqlite3_errmsg(sqlite3_db_handle(m_stmt)) + ")");
    }

    return *this;
}

int Statement::Index(const std::string& param) const
{
    const int index = sqlite3_bind_parameter_index(m_stmt, param.c_str());

    if (index == 0)
    {
        throw std::runtime_error("No parameter named " + param);
    }

    return index;
}

Statement Statement::Prepare(sqlite3 *db, std::string_view sql)
{
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.data(), static_cast<int>(sql.size()), &stmt, nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to prepare SQLite statement: " << sqlite3_errmsg(db);
        throw std::runtime_error("Failed to prepare SQLite statement: " + std::string(sqlite3_errmsg(db)));
    }

    return Statement(stmt);
}

Statement& Statement::Bind(const std::string& portal, bool value)
{
    return Check(sqlite3_bind_int(m_stmt, Index(portal), value ? 1 : 0), portal);
}

Statement& Statement::Bind(const std::string& portal, int value)
{
    return Check(sqlite3_bind_int(m_stmt, Index(portal), value), portal);
}

Statement& Statement::Bind(const std::string& portal, std::int64_t value)
{
    return Check(sqlite3_bind_int64(m_stmt, Index(portal), value), portal);
}

Statement& Statement::Bind(const std::string& portal, std::uint64_t value)
{
    return Check(sqlite3_bind_int64(m_stmt, Index(portal), static_cast<std::int64_t>(value)), portal);
}

Statement& Statement::Bind(const std::string& portal, std::string_view value)
{
    return Check(
        sqlite3_bind_text(
            m_stmt,
            Index(portal),
            value.data() != nullptr ? value.data() : "",
            static_cast<int>(value.size()),
            SQLITE_TRANSIENT),
        portal);
}

Statement& Statement::Bind(const std::string& portal, const char* value)
{
    return Bind(portal, std::string_view(value));
}

Statement& Statement::Bind(const std::string& portal, const std::vector<char>& buffer)
{
    return Check(
        sqlite3_bind_blob(
            m_stmt,
            Index(portal),
            buffer.empty() ? "" : buffer.data(),
            static_cast<int>(buffer.size()),
            SQLITE_TRANSIENT),
        portal);
}

Statement& Statement::Bind(const std::string& portal, std::nullopt_t)
{
    return Check(sqlite3_bind_null(m_stmt, Index(portal)), portal);
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
    std::map<std::string, int> cols;

    for (int i = 0; i < sqlite3_column_count(m_stmt); i++)
    {
        cols.insert({ sqlite3_column_name(m_stmt, i), i });
    }

    do
    {
        switch (int res = sqlite3_step(m_stmt))
        {
        case SQLITE_DONE:
            return;
        case SQLITE_ROW:
        {
            InternalRow r(m_stmt, cols);
            cb(r);
            break;
        }
        default:
            BOOST_LOG_TRIVIAL(error) << "Unexpected SQLite return code " << res;
            throw std::runtime_error("Unexpected SQLite return code");
        }
    } while (true);
}
