#include "../migrations.hpp"

#include <boost/log/trivial.hpp>

int porla::Data::Migrations::Tokens(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Adding table 'tokens' (user sessions and API keys)";

    return sqlite3_exec(
        db,
        R"sql(
        CREATE table tokens (
            id             TEXT PRIMARY KEY,
            kind           TEXT NOT NULL CHECK (kind in ('api_key', 'session')),
            user_id        INTEGER NULL REFERENCES users(id) ON DELETE CASCADE,
            name           TEXT NULL,
            secret_hash    BLOB NOT NULL,
            created_at     INTEGER NOT NULL,
            expires_at     INTEGER NULL,
            last_used_at   INTEGER NULL,
            user_agent     TEXT NULL,
            remote_address TEXT NULL
        );

        INSERT INTO tokens (id, kind, name, secret_hash, created_at, expires_at)
        SELECT id, 'api_key', name, secret_hash, created_at, expires_at FROM apikeys;

        DROP TABLE apikeys;
        )sql",
        nullptr,
        nullptr,
        nullptr);
}
