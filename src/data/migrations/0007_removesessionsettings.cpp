#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::RemoveSessionSettings(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Removing 'sessionsettings' table";

    return sqlite3_exec(
            db,
            "DROP TABLE sessionsettings",
            nullptr,
            nullptr,
            nullptr);
}
