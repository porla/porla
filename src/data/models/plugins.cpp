#include "plugins.hpp"

#include "../statement.hpp"

using porla::Data::Models::Plugins;

const std::string PluginsSelectPrefix = R"sql(
    SELECT
        id,
        path,
        config,
        metadata
    FROM plugins
)sql";

static Plugins::Plugin LoadFromRow(const porla::Data::Statement::IRow &row)
{
    const auto metadata = row.GetStdString("metadata");
    const auto metadata_json = nlohmann::json::parse(metadata);

    return Plugins::Plugin{
        .id = row.GetInt32("id"),
        .path = row.GetStdString("path"),
        .config = row.GetOptionalStdString("config"),
        .metadata = metadata_json
    };
}

std::optional<Plugins::Plugin> Plugins::GetById(sqlite3* db, int id)
{
    std::optional<Plugin> plugin = std::nullopt;

    Statement::Prepare(db, PluginsSelectPrefix + "WHERE id = $id")
        .Bind("$id", id)
        .Step([&plugin](const auto& row)
        {
            plugin = LoadFromRow(row);
            return SQLITE_OK;
        });

    return plugin;
}

int Plugins::Insert(sqlite3* db, const Plugins::Plugin& plugin)
{
    const auto metadata_json = nlohmann::json(plugin.metadata).dump();

    auto stmt = Statement::Prepare(
        db,
        R"sql(
        INSERT INTO plugins (path, config, metadata)
        VALUES ($path, $config, $metadata);
        )sql");
    stmt.Bind("$path", plugin.path);
    stmt.Bind("$config", plugin.config);
    stmt.Bind("$metadata", metadata_json);
    stmt.Execute();

    return sqlite3_last_insert_rowid(db);
}

std::vector<Plugins::Plugin> Plugins::List(sqlite3* db)
{
    std::vector<Plugin> plugins;

    Statement::Prepare(db, PluginsSelectPrefix + " ORDER BY id ASC")
        .Step([&plugins](const auto& row)
        {
            plugins.emplace_back(LoadFromRow(row));
            return SQLITE_OK;
        });

    return plugins;
}

void Plugins::Remove(sqlite3* db, int id)
{
}

void Plugins::Update(sqlite3* db, const Plugin& plugin)
{
}
