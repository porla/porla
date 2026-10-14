#include "../migrations.hpp"

#include <boost/log/trivial.hpp>

int porla::Data::Migrations::Users(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Creating 'users' table";

    return sqlite3_exec(
        db,
        "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY,"
        "username TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL"
        ");",
        nullptr,
        nullptr,
        nullptr);
}
