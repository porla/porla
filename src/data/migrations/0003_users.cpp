#include "0003_users.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::Users;

int Users::Migrate(sqlite3* db)
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
