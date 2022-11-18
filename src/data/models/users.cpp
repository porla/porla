#include "users.hpp"

#include "../statement.hpp"

using porla::Data::Models::Users;

bool Users::Any(sqlite3* db)
{
    bool any = false;

    Statement::Prepare(db, "SELECT COUNT(*) FROM users")
        .Step(
            [&any](auto const& row)
            {
                any = row.GetInt32(0) > 0;
                return SQLITE_OK;
            });

    return any;
}

std::optional<Users::User> Users::GetByUsername(sqlite3* db, const std::string& username)
{
    std::optional<User> user;

    Statement::Prepare(db, "SELECT username,password FROM users WHERE username = $1")
        .Bind(1, std::string_view(username))
        .Step(
            [&user](auto const& row)
            {
                user = User{
                    .username        = row.GetStdString(0),
                    .password_hashed = row.GetStdString(1)
                };

                return SQLITE_OK;
            });

    return user;
}

void Users::Insert(sqlite3* db, const porla::Data::Models::Users::User &user)
{
    Statement::Prepare(db, "INSERT INTO users (username, password) VALUES ($1, $2);")
        .Bind(1, std::string_view(user.username))
        .Bind(2, std::string_view(user.password_hashed))
        .Execute();
}
