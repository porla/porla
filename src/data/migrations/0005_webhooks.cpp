#include "0005_webhooks.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::Webhooks;

int Webhooks::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Creating 'webhooks' table";

    int res = sqlite3_exec(
        db,
        "CREATE TABLE webhooks ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "event TEXT NOT NULL,"
        "method TEXT NOT NULL,"
        "url TEXT NOT NULL,"
        "headers TEXT NOT NULL,"
        "payload TEXT NOT NULL"
        ");",
        nullptr,
        nullptr,
        nullptr);

    return res;
}
