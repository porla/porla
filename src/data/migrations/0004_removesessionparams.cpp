#include "../migrations.hpp"

#include <boost/log/trivial.hpp>

int porla::Data::Migrations::RemoveSessionParams(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Removing 'sessionparams' table";
    return sqlite3_exec(db, "DROP TABLE sessionparams;", nullptr, nullptr, nullptr);
}
