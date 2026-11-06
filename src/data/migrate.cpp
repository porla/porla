#include "migrate.hpp"

#include <functional>
#include <string>
#include <vector>

#include "migrations.hpp"
#include "statement.hpp"
#include "transaction.hpp"

#include "../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

int GetUserVersion(sqlite3* db)
{
    return porla::Data::Statement::Prepare(db, "PRAGMA user_version;")
        .One([](const auto& row) { return row.GetInt32("user_version"); })
        .value_or(0);
}

int SetUserVersion(sqlite3* db, int version)
{
    std::string sql = "PRAGMA user_version = " + std::to_string(version) + ";";
    return sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
}

bool porla::Data::Migrate(sqlite3* db)
{
    static std::vector<std::function<int(sqlite3*)>> Migrations =
    {
        &porla::Data::Migrations::InitialSetup,
        &porla::Data::Migrations::AddSessionSettings,
        &porla::Data::Migrations::Users,
        &porla::Data::Migrations::RemoveSessionParams,
        &porla::Data::Migrations::TorrentsMetadata,
        &porla::Data::Migrations::ClientData,
        &porla::Data::Migrations::RemoveSessionSettings,
        &porla::Data::Migrations::Plugins,
        &porla::Data::Migrations::MultiSessions,
        &porla::Data::Migrations::Sessions,
        &porla::Data::Migrations::Presets,
        &porla::Data::Migrations::AlterPlugins,
        &porla::Data::Migrations::AddKeyValueStore,
        &porla::Data::Migrations::MorePresets,
        &porla::Data::Migrations::AddTorrentParamsIndex,
        &porla::Data::Migrations::AddApiKeys,
        &porla::Data::Migrations::TorrentInfos,
        &porla::Data::Migrations::AddCompletedAt,
        &porla::Data::Migrations::AddAutoIncrementIds,
        &porla::Data::Migrations::Tokens,
    };

    int user_version = GetUserVersion(db);

    if (user_version > Migrations.size())
    {
        BOOST_LOG_SEV(logger::get(), Log::error)
            << Log::Property("version", user_version)
            << "Unsupported database version";

        return false;
    }

    if (user_version == Migrations.size())
    {
        return true;
    }

    PORLA_LOG(logger::get(), info,
        "Migrating database from version {from_version} to {to_version}", user_version, Migrations.size());

    Transaction tx(db);

    for (int i = user_version; i < Migrations.size(); i++)
    {
        try
        {
            const auto result = Migrations.at(i)(db);

            if (result == SQLITE_OK)
            {
                continue;
            }

            BOOST_LOG_SEV(logger::get(), Log::error)
                << Log::Property("error", sqlite3_errmsg(db))
                << "Failed to apply migration " << i + 1;
        }
        catch(const std::exception& e)
        {
            BOOST_LOG_SEV(logger::get(), Log::error)
                << Log::Property("error", e.what())
                << "Failed to apply migration " << i + 1;
        }

        tx.Rollback();

        return false;
    }

    if (SetUserVersion(db, static_cast<int>(Migrations.size())) != SQLITE_OK)
    {
        BOOST_LOG_SEV(logger::get(), Log::error)
            << Log::Property("error", sqlite3_errmsg(db))
            << "Failed to set user version";

        tx.Rollback();

        return false;
    }

    if (!tx.Commit())
    {
        BOOST_LOG_SEV(logger::get(), Log::error)
            << "Failed to commit migration transaction";

        return false;
    }

    return true;
}
