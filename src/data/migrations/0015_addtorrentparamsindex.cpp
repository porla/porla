#include "../migrations.hpp"

#include <boost/log/trivial.hpp>

int porla::Data::Migrations::AddTorrentParamsIndex(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Adding index to 'addtorrentparams' table";

    return sqlite3_exec(
        db,
        R"sql(
        CREATE INDEX ix_addtorrentparams_session_queue
            ON addtorrentparams (session_id, queue_position, id);
        )sql",
        nullptr,
        nullptr,
        nullptr);
}
