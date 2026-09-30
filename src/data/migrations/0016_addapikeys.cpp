#include "0016_addapikeys.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::AddApiKeys;

int AddApiKeys::Migrate(sqlite3* db)
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
            expired_at  INTEGER
        );
        )sql",
        nullptr,
        nullptr,
        nullptr);
}
