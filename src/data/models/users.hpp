#pragma once

#include <optional>
#include <string>

#include <sqlite3.h>

namespace porla::Data::Models
{
    class Users
    {
    public:
        struct User
        {
            std::string username;
            std::string password_hashed;
        };

        static bool Any(sqlite3* db);
        static std::optional<User> GetByUsername(sqlite3* db, const std::string& username);
        static void Insert(sqlite3* db, const User& user);
    };
}
