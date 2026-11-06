#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::AddTorrentParamsIndex(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Adding index to 'addtorrentparams' table";

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
