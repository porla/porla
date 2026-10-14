#include "../migrations.hpp"

#include <boost/log/trivial.hpp>

int porla::Data::Migrations::RemoveSessionSettings(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Removing 'sessionsettings' table";

    return sqlite3_exec(
            db,
            "DROP TABLE sessionsettings",
            nullptr,
            nullptr,
            nullptr);
}
