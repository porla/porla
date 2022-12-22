#include "0005_metadata.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::TorrentsMetadata;

int TorrentsMetadata::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Creating 'torrentsmetadata' table";

    int res = sqlite3_exec(
        db,
        "CREATE TABLE torrentsmetadata ("
            "id INTEGER PRIMARY KEY,"
            "info_hash_v1 TEXT,"
            "info_hash_v2 TEXT,"
            "key TEXT NOT NULL,"
            "value TEXT NOT NULL,"
            "CHECK (info_hash_v1 IS NOT NULL OR info_hash_v2 IS NOT NULL),"
            "UNIQUE (info_hash_v1, info_hash_v2, key)"
        ");",
        nullptr,
        nullptr,
        nullptr);

    return res;
}
