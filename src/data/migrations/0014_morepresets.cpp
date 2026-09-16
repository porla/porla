#include "0014_morepresets.hpp"

#include <boost/log/trivial.hpp>
#include <sqlite3.h>

using porla::Data::Migrations::MorePresets;

int MorePresets::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Adding columns to 'presets' table";

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
