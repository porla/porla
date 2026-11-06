#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::TorrentsMetadata(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Creating 'torrentsmetadata' table";

    return sqlite3_exec(
        db,
        "CREATE TABLE torrentsmetadata ("
            "id INTEGER PRIMARY KEY,"
            "info_hash_v1 TEXT,"
            "info_hash_v2 TEXT,"
            "key TEXT NOT NULL,"
            "value TEXT NOT NULL,"
            "CHECK (info_hash_v1 IS NOT NULL OR info_hash_v2 IS NOT NULL),"
            "UNIQUE (info_hash_v1, info_hash_v2, key)"
        ");",
        nullptr,
        nullptr,
        nullptr);
}
