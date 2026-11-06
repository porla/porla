#include "../migrations.hpp"

#include "../../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int porla::Data::Migrations::MorePresets(sqlite3* db)
{
    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Adding columns to 'presets' table";

    return sqlite3_exec(
        db,
        R"sql(
        ALTER TABLE presets ADD COLUMN flags      INTEGER;
        ALTER TABLE presets ADD COLUMN flags_mask INTEGER;
        )sql",
        nullptr,
        nullptr,
        nullptr);
}
