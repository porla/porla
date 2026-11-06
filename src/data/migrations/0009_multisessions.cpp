#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::MultiSessions(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Adding 'session_id' column to table 'addtorrentparams'";

    return sqlite3_exec(
        db,
        "ALTER TABLE addtorrentparams ADD COLUMN session_id TEXT NOT NULL DEFAULT 'default'",
        nullptr,
        nullptr,
        nullptr);
}
