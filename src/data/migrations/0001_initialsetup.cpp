#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::InitialSetup(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Creating 'addtorrentparams' table";

    int res = sqlite3_exec(
        db,
        "CREATE TABLE addtorrentparams ("
            "id INTEGER PRIMARY KEY,"
            "info_hash_v1 TEXT,"
            "info_hash_v2 TEXT,"
            "name TEXT,"
            "queue_position INTEGER NOT NULL,"
            "resume_data_buf BLOB NOT NULL,"
            "save_path TEXT NOT NULL,"
            "CHECK (info_hash_v1 IS NOT NULL OR info_hash_v2 IS NOT NULL),"
            "UNIQUE (info_hash_v1),"
            "UNIQUE (info_hash_v2)"
        ");",
        nullptr,
        nullptr,
        nullptr);

    if (res != SQLITE_OK) { return res; }

    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Creating 'sessionparams' table";

    return sqlite3_exec(
        db,
        "CREATE TABLE sessionparams ("
            "data BLOB NOT NULL,"
            "timestamp INTEGER NOT NULL DEFAULT (strftime('%s'))"
        ");",
        nullptr,
        nullptr,
        nullptr);
}
