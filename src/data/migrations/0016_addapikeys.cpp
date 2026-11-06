#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::AddApiKeys(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Adding table 'apikeys'";

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
