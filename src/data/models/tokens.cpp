#include "tokens.hpp"

#include "../statement.hpp"

using porla::Data::Models::Tokens;
using porla::Data::Statement;

namespace
{
    const std::string kSelect = R"sql(
    SELECT t.id, t.kind, t.user_id, u.username, t.name, t.secret_hash, t.created_at, t.expires_at, t.last_used_at, t.user_agent, t.remote_address
    FROM tokens t
    LEFT JOIN users u ON u.id = t.user_id
    )sql";

    Tokens::Token LoadFromRow(const Statement::IRow& row)
    {
        return Tokens::Token{
            .id = row.GetStdString("id"),
            .kind = row.GetStdString("kind"),
            .user_id = row.GetOptionalInt32("user_id"),
            .username = row.GetOptionalStdString("username"),
            .name = row.GetOptionalStdString("name"),
            .secret_hash = row.GetBuffer("secret_hash"),
            .created_at = row.GetInt64("created_at"),
            .expires_at = row.GetOptionalInt64("expires_at"),
            .last_used_at = row.GetOptionalInt64("last_used_at"),
            .user_agent = row.GetOptionalStdString("user_agent"),
            .remote_address = row.GetOptionalStdString("remote_address")
        };
    }
}

std::optional<Tokens::Token> Tokens::GetById(sqlite3* db, std::string_view id)
{
    return Statement::Prepare(db, kSelect + " WHERE t.id = $id")
        .Bind("$id", id)
        .One(LoadFromRow);
}

void Tokens::Insert(sqlite3* db, const Token& token)
{
    Statement::Prepare(
        db,
        R"sql(
        INSERT INTO tokens (
            id, kind, user_id, name, secret_hash, created_at, expires_at, last_used_at, user_agent, remote_address
        )
        VALUES (
            $id, $kind, $user_id, $name, $secret_hash, $created_at, $expires_at, $last_used_at, $user_agent, $remote_address
        );
        )sql")
        .Bind("$id", token.id)
        .Bind("$kind", token.kind)
        .Bind("$user_id", token.user_id)
        .Bind("$name", token.name)
        .Bind("$secret_hash", token.secret_hash)
        .Bind("$created_at", token.created_at)
        .Bind("$expires_at", token.expires_at)
        .Bind("$last_used_at", token.last_used_at)
        .Bind("$user_agent", token.user_agent)
        .Bind("$remote_address", token.remote_address)
        .Execute();
}

std::vector<Tokens::Token> Tokens::List(sqlite3* db, std::string_view kind)
{
    return Statement::Prepare(db, kSelect + " WHERE t.kind = $kind ORDER BY t.name ASC, t.created_at ASC")
        .Bind("$kind", kind)
        .All(LoadFromRow);
}

void Tokens::Remove(sqlite3* db, std::string_view id)
{
    Statement::Prepare(db, "DELETE FROM tokens WHERE id = $id")
        .Bind("$id", id)
        .Execute();
}

int Tokens::RemoveExpiredSessions(sqlite3* db, std::int64_t now)
{
    // expired API keys stay listed (auth.keys.list shows them) - only sessions are cleaned up
    Statement::Prepare(db, "DELETE FROM tokens WHERE kind = 'session' AND expires_at < $now")
        .Bind("$now", now)
        .Execute();

    return sqlite3_changes(db);
}

int Tokens::RemoveLeastRecentlyUsedSessions(sqlite3* db, int user_id, int keep)
{
    Statement::Prepare(
        db,
        R"sql(
        DELETE FROM tokens
        WHERE kind = 'session' AND user_id = $user_id AND id NOT IN (
            SELECT id FROM tokens
            WHERE kind = 'session' AND user_id = $user_id
            ORDER BY COALESCE(last_used_at, created_at) DESC, created_at DESC
            LIMIT $keep
        )
        )sql")
        .Bind("$user_id", user_id)
        .Bind("$keep", keep)
        .Execute();

    return sqlite3_changes(db);
}

void Tokens::Touch(sqlite3* db, std::string_view id, std::int64_t now, std::optional<std::int64_t> expires_at)
{
    Statement::Prepare(db, "UPDATE tokens SET last_used_at = $now, expires_at = COALESCE($expires_at, expires_at) WHERE id = $id")
        .Bind("$id", id)
        .Bind("$now", now)
        .Bind("$expires_at", expires_at)
        .Execute();
}
