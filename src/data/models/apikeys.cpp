#include "apikeys.hpp"

#include "../statement.hpp"

using porla::Data::Models::ApiKeys;
using porla::Data::Statement;

namespace
{
    ApiKeys::ApiKey LoadFromRow(const Statement::IRow& row)
    {
        return ApiKeys::ApiKey{
            .id         = row.GetStdString("id"),
            .name       = row.GetStdString("name"),
            .created_at = row.GetInt64("created_at"),
            .expires_at = row.GetOptionalInt64("expires_at")
        };
    }
}

std::optional<std::vector<char>> ApiKeys::GetSecretHashById(sqlite3* db, std::string_view id)
{
    std::optional<std::vector<char>> result;

    Statement::Prepare(db, "SELECT secret_hash FROM apikeys WHERE id = $id")
        .Bind("$id", id)
        .Step([&result](const Statement::IRow& row)
        {
            result = row.GetBuffer("secret_hash");
            return SQLITE_OK;
        });

    return result;
}

void ApiKeys::Insert(
    sqlite3* db,
    std::string_view id,
    std::string_view name,
    std::string_view secret_hash,
    std::optional<std::int64_t> expires_at)
{
    auto stmt = Statement::Prepare(
        db,
        R"sql(
        INSERT INTO apikeys (
            id,
            name,
            secret_hash,
            created_at,
            expires_at
        )
        VALUES (
            $id,
            $name,
            $secret_hash,
            strftime('%s', 'now'),
            $expires_at
        );
        )sql");

    stmt.Bind("$id", id);
    stmt.Bind("$name", name);
    stmt.Bind("$secret_hash", secret_hash);
    stmt.Bind("$expires_at", expires_at);
    stmt.Execute();
}

std::vector<ApiKeys::ApiKey> ApiKeys::List(sqlite3* db)
{
    std::vector<ApiKey> result;

    auto stmt = Statement::Prepare(
        db,
        R"sql(
        SELECT id,name,created_at,expires_at FROM apikeys ORDER BY name ASC
        )sql");

    stmt.Step([&result](const auto& row)
    {
        result.emplace_back(LoadFromRow(row));
        return SQLITE_OK;
    });

    return result;
}

void ApiKeys::Remove(sqlite3* db, std::string_view id)
{
    Statement::Prepare(
        db,
        R"sql(
        DELETE FROM apikeys WHERE id = $id;
        )sql")
        .Bind("$id", id)
        .Execute();
}
