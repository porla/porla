#include "0004_removesessionparams.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::RemoveSessionParams;

int RemoveSessionParams::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Removing 'sessionparams' table";
    return sqlite3_exec(db, "DROP TABLE sessionparams;", nullptr, nullptr, nullptr);
}
