#include "0009_multisessions.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::MultiSessions;

int MultiSessions::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Adding 'session_id' column to table 'addtorrentparams'";

    int res = sqlite3_exec(
        db,
        "ALTER TABLE addtorrentparams ADD COLUMN session_id TEXT NOT NULL DEFAULT 'default'",
        nullptr,
        nullptr,
        nullptr);

    return res;
}
