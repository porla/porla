#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::Users(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Creating 'users' table";

    return sqlite3_exec(
        db,
        "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY,"
        "username TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL"
        ");",
        nullptr,
        nullptr,
        nullptr);
}
