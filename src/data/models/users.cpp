#include "users.hpp"

#include "../statement.hpp"

using porla::Data::Models::Users;

bool Users::Any(sqlite3* db)
{
    bool any = false;

    Statement::Prepare(db, "SELECT COUNT(*) AS count FROM users")
        .Step(
            [&any](auto const& row)
            {
                any = row.GetInt32("count") > 0;
                return SQLITE_OK;
            });

    return any;
}

std::optional<Users::User> Users::GetByUsername(sqlite3* db, const std::string& username)
{
    std::optional<User> user;

    Statement::Prepare(db, "SELECT username,password FROM users WHERE username = $username")
        .Bind("$username", username)
        .Step(
            [&user](auto const& row)
            {
                user = User{
                    .username        = row.GetStdString("username"),
                    .password_hashed = row.GetStdString("password")
                };

                return SQLITE_OK;
            });

    return user;
}

void Users::Insert(sqlite3* db, const porla::Data::Models::Users::User &user)
{
    Statement::Prepare(db, "INSERT INTO users (username, password) VALUES ($username, $password);")
        .Bind("$username", user.username)
        .Bind("$password", user.password_hashed)
        .Execute();
}
