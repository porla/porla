#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::AddKeyValueStore(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Adding 'kvs' table";

    return sqlite3_exec(
        db,
        R"sql(
        CREATE TABLE kvs (
            id INTEGER PRIMARY KEY,
            key TEXT NOT NULL UNIQUE,
            value BLOB NOT NULL,
            readonly INTEGER NOT NULL
        );
        )sql",
        nullptr,
        nullptr,
        nullptr);
}
