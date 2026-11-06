#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::Plugins(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Creating 'plugins' table";

    return sqlite3_exec(
        db,
        "CREATE TABLE plugins ("
            "id INTEGER PRIMARY KEY,"
            "name TEXT NOT NULL UNIQUE,"
            "path TEXT NOT NULL UNIQUE,"
            "config TEXT,"
            "enabled INTEGER NOT NULL DEFAULT 0"
        ");",
        nullptr,
        nullptr,
        nullptr);
}
