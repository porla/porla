#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::ClientData(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Adding client_data column to addtorrentparams table";

    int res = sqlite3_exec(
        db,
        "ALTER TABLE addtorrentparams ADD COLUMN client_data TEXT NULL;",
        nullptr,
        nullptr,
        nullptr);

    if (res != SQLITE_OK)
    {
        return res;
    }

    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Dropping torrentsmetadata table";

    return sqlite3_exec(
        db,
        "DROP TABLE torrentsmetadata;",
        nullptr,
        nullptr,
        nullptr);
}
