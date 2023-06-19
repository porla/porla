#include "0008_plugins.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::Plugins;

int Plugins::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Creating 'plugins' table";

    int res = sqlite3_exec(
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

    return res;
}
