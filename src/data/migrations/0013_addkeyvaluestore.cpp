#include "../migrations.hpp"

#include <boost/log/trivial.hpp>

int porla::Data::Migrations::AddKeyValueStore(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Adding 'kvs' table";

    int res = sqlite3_exec(
        db,
        R"sql(
        CREATE TABLE kvs (
            id INTEGER PRIMARY KEY,
            key TEXT NOT NULL UNIQUE,
            value BLOB NOT NULL,
            readonly INTEGER NOT NULL
        );
        )sql",
        nullptr,
        nullptr,
        nullptr);

    return res;
}
