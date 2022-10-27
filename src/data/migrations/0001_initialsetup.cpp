#include "0001_initialsetup.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::InitialSetup;

int InitialSetup::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Creating 'addtorrentparams' table";

    int res = sqlite3_exec(
        db,
        "CREATE TABLE addtorrentparams ("
            "id INTEGER PRIMARY KEY,"
            "info_hash_v1 TEXT,"
            "info_hash_v2 TEXT,"
            "name TEXT,"
            "queue_position INTEGER NOT NULL,"
            "resume_data_buf BLOB NOT NULL,"
            "save_path TEXT NOT NULL,"
            "CHECK (info_hash_v1 IS NOT NULL OR info_hash_v2 IS NOT NULL),"
            "UNIQUE (info_hash_v1),"
            "UNIQUE (info_hash_v2)"
        ");",
        nullptr,
        nullptr,
        nullptr);

    if (res != SQLITE_OK) { return res; }

    BOOST_LOG_TRIVIAL(info) << "Creating 'sessionparams' table";

    return sqlite3_exec(
        db,
        "CREATE TABLE sessionparams ("
            "data BLOB NOT NULL,"
            "timestamp INTEGER NOT NULL DEFAULT (strftime('%s'))"
        ");",
        nullptr,
        nullptr,
        nullptr);
}
