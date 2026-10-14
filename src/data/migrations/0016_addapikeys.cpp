#include "../migrations.hpp"

#include <boost/log/trivial.hpp>

int porla::Data::Migrations::AddApiKeys(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Adding table 'apikeys'";

    return sqlite3_exec(
        db,
        R"sql(
        CREATE TABLE apikeys (
            id          TEXT PRIMARY KEY,
            name        TEXT NOT NULL,
            secret_hash BLOB NOT NULL,
            created_at  INTEGER NOT NULL,
            expires_at  INTEGER
        );
        )sql",
        nullptr,
        nullptr,
        nullptr);
}
