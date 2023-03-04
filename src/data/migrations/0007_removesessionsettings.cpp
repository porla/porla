#include "0007_removesessionsettings.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::RemoveSessionSettings;

int RemoveSessionSettings::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Removing 'sessionsettings' table";

    return sqlite3_exec(
            db,
            "DROP TABLE sessionsettings",
            nullptr,
            nullptr,
            nullptr);
}
