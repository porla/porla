#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::AddSessionSettings(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Creating 'sessionsettings' table";

    return sqlite3_exec(
        db,
        "CREATE TABLE sessionsettings ("
        "id INTEGER PRIMARY KEY,"
        "key TEXT NOT NULL,"
        "value TEXT NOT NULL,"
        "UNIQUE (key)"
        ");",
        nullptr,
        nullptr,
        nullptr);
}
