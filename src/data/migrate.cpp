#include "migrate.hpp"

#include <functional>
#include <string>
#include <vector>

#include <boost/log/trivial.hpp>

#include "../config.hpp"
#include "migrations/0001_initialsetup.hpp"
#include "migrations/0002_addsessionsettings.hpp"
#include "migrations/0003_users.hpp"
#include "migrations/0004_removesessionparams.hpp"
#include "migrations/0005_metadata.hpp"
#include "migrations/0006_clientdata.hpp"
#include "migrations/0007_removesessionsettings.hpp"
#include "migrations/0008_plugins.hpp"
#include "migrations/0009_multisessions.hpp"
#include "migrations/0010_sessions.hpp"
#include "migrations/0011_presets.hpp"
#include "migrations/0012_alterplugins.hpp"
#include "migrations/0013_addkeyvaluestore.hpp"
#include "statement.hpp"

int GetUserVersion(sqlite3* db)
{
    int version = 0;

    porla::Data::Statement::Prepare(db, "PRAGMA user_version;")
        .Step([&version](const auto& row)
        {
            version = row.GetInt32("user_version");
            return SQLITE_OK;
        });

    return version;
}

void SetUserVersion(sqlite3* db, int version)
{
    std::string sql = "PRAGMA user_version = " + std::to_string(version) + ";";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
}

bool porla::Data::Migrate(sqlite3* db, const std::unique_ptr<porla::Config>& cfg)
{
    static std::vector<std::function<int(sqlite3*)>> Migrations =
    {
        &porla::Data::Migrations::InitialSetup::Migrate,
        &porla::Data::Migrations::AddSessionSettings::Migrate,
        &porla::Data::Migrations::Users::Migrate,
        &porla::Data::Migrations::RemoveSessionParams::Migrate,
        &porla::Data::Migrations::TorrentsMetadata::Migrate,
        &porla::Data::Migrations::ClientData::Migrate,
        &porla::Data::Migrations::RemoveSessionSettings::Migrate,
        &porla::Data::Migrations::Plugins::Migrate,
        &porla::Data::Migrations::MultiSessions::Migrate,
        [&cfg](sqlite3* db) { return porla::Data::Migrations::Sessions::Migrate(db, cfg); },
        [&cfg](sqlite3* db) { return porla::Data::Migrations::Presets::Migrate(db, cfg); },
        &porla::Data::Migrations::AlterPlugins::Migrate,
        &porla::Data::Migrations::AddKeyValueStore::Migrate,
    };

    int user_version = GetUserVersion(db);

    if (user_version > Migrations.size())
    {
        BOOST_LOG_TRIVIAL(error) << "Unsupported database version: " << user_version;
        return false;
    }

    if (user_version == Migrations.size())
    {
        return true;
    }

    BOOST_LOG_TRIVIAL(info) << "Migrating database from version " << user_version + 1 << " to " << Migrations.size();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    for (int i = user_version; i < Migrations.size(); i++)
    {
        if (Migrations.at(i)(db) != SQLITE_OK)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to apply migration " << i + 1 << ": " << sqlite3_errmsg(db);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }
    }

    SetUserVersion(db, static_cast<int>(Migrations.size()));

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    return true;
}
