#include "migrate.hpp"

#include <functional>
#include <string>
#include <vector>

#include "migrations/0001_initialsetup.hpp"
#include "migrations/0002_addsessionsettings.hpp"
#include "statement.hpp"

int GetUserVersion(sqlite3* db)
{
    int version = 0;

    porla::Data::Statement::Prepare(db, "PRAGMA user_version;")
        .Step([&version](const auto& row)
        {
            version = row.GetInt32(0);
            return SQLITE_OK;
        });

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
        &porla::Data::Migrations::InitialSetup::Migrate,
        &porla::Data::Migrations::AddSessionSettings::Migrate
    };

    for (int i = GetUserVersion(db); i < Migrations.size(); i++)
    {
        if (Migrations.at(i)(db) != SQLITE_OK)
        {
            return false;
        }
    }

    SetUserVersion(db, static_cast<int>(Migrations.size()));

    return true;
}
