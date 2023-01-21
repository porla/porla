#include "0006_clientdata.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::ClientData;

int ClientData::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Adding client_data column to addtorrentparams table";

    return sqlite3_exec(
        db,
        "ALTER TABLE addtorrentparams ADD COLUMN client_data TEXT NULL;",
        nullptr,
        nullptr,
        nullptr);
}
