#include "migrate.hpp"

#include <functional>
#include <string>
#include <vector>

#include "migrations/0001_initialsetup.hpp"

int GetUserVersion(sqlite3* db)
{
    int version = 0;

    sqlite3_exec(
        db,
        "PRAGMA user_version;",
        [](void* user, int, char** d, char** a)
        {
            int* v = reinterpret_cast<int*>(user);
            *v = std::stoi(d[0]);
            return SQLITE_OK;
        },
        &version,
        nullptr);

    return version;
}

void SetUserVersion(sqlite3* db, int version)
{
    std::string sql = "PRAGMA user_version = " + std::to_string(version) + ";";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
}

bool porla::Data::Migrate(sqlite3* db)
{
    static std::vector<std::function<int(sqlite3*)>> Migrations =
    {
        &porla::Data::Migrations::InitialSetup::Migrate
    };

    for (int i = GetUserVersion(db); i < Migrations.size(); i++)
    {
        Migrations.at(i)(db);
    }

    SetUserVersion(db, static_cast<int>(Migrations.size()));

    return true;
}
