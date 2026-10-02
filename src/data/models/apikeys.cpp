#include "apikeys.hpp"

#include "../statement.hpp"

using porla::Data::Models::ApiKeys;
using porla::Data::Statement;

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
