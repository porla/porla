#include "0002_addsessionsettings.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::AddSessionSettings;

int AddSessionSettings::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Creating 'sessionsettings' table";

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
