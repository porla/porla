#include "presets.hpp"

#include <nlohmann/json.hpp>

#include "../statement.hpp"

using json = nlohmann::json;
using porla::Data::Models::Presets;

const std::string PresetSelectPrefix = R"sql(
    SELECT
        id,
        name,
        is_default,
        category,
        download_limit,
        max_connections,
        max_uploads,
        metadata,
        session_id,
        save_path,
        storage_mode,
        tags,
        upload_limit
    FROM presets
)sql";

static Presets::Preset LoadFromRow(const porla::Data::Statement::IRow &row)
{
    const auto metadata = row.GetOptionalStdString("metadata");
    const auto tags     = row.GetOptionalStdString("tags");

    return Presets::Preset{
        .id = row.GetInt32("id"),
        .name = row.GetStdString("name"),
        .is_default = row.GetInt32("is_default") == 1,
        .category = row.GetOptionalStdString("category"),
        .download_limit = row.GetOptionalInt32("download_limit"),
        .max_connections = row.GetOptionalInt32("max_connections"),
        .max_uploads = row.GetOptionalInt32("max_uploads"),
        .metadata = metadata.has_value()
            ? std::optional(json::parse(metadata.value()).get<std::map<std::string, json>>())
            : std::nullopt,
        .session_id = row.GetOptionalInt32("session_id"),
        .save_path = row.GetOptionalStdString("save_path"),
        .storage_mode = row.GetOptionalStdString("storage_mode"),
        .tags = tags.has_value()
            ? json::parse(tags.value()).get<std::unordered_set<std::string>>()
            : std::unordered_set<std::string>(),
        .upload_limit = row.GetOptionalInt32("upload_limit")
    };
}

void Presets::ForEach(sqlite3 *db, const std::function<void(const Presets::Preset &)> &cb)
{
    Statement::Prepare(db, PresetSelectPrefix + " ORDER BY name ASC")
        .Step(
            [&cb](auto const &row)
            {
                cb(LoadFromRow(row));
                return SQLITE_OK;
            });
}

std::optional<Presets::Preset> Presets::GetById(sqlite3 *db, int id)
{
    std::optional<Preset> preset;

    Statement::Prepare(db, PresetSelectPrefix + " WHERE id = $id")
        .Bind("$id", id)
        .Step(
            [&preset](auto const &row)
            {
                preset = LoadFromRow(row);
                return SQLITE_OK;
            });

    return preset;
}

std::optional<Presets::Preset> Presets::GetDefault(sqlite3 *db)
{
    std::optional<Preset> preset;

    Statement::Prepare(db, PresetSelectPrefix + " WHERE is_default = 1")
        .Step(
            [&preset](auto const &row)
            {
                preset = LoadFromRow(row);
                return SQLITE_OK;
            });

    return preset;
}

int Presets::Insert(sqlite3 *db, const std::string& name)
{
    auto stmt = Statement::Prepare(db, "INSERT INTO presets (name) VALUES ($name);");
    stmt.Bind("$name", name);
    stmt.Execute();

    return sqlite3_last_insert_rowid(db);
}

void Presets::Remove(sqlite3* db, int id)
{
    auto stmt = Statement::Prepare(db, "DELETE FROM presets WHERE id = $id");
    stmt.Bind("$id", id);
    stmt.Execute();
}

void Presets::Update(sqlite3 *db, const Presets::Preset &preset)
{
    std::optional<std::string> metadata;
    std::optional<std::string> tags = json(preset.tags).dump();

    if (preset.metadata.has_value())
    {
        metadata = json(preset.metadata.value()).dump();
    }

    const auto current_default = GetDefault(db);

    if (preset.is_default && current_default.has_value() && current_default->id != preset.id)
    {
        // The current default is different than ours. Remove it from our default
        // and let the update below set ours as default.
        Statement::Prepare(db, "UPDATE presets SET is_default = 0")
            .Execute();
    }

    auto stmt = Statement::Prepare(
        db,
        R"sql(
        UPDATE
            presets
        SET
            name            = $name,
            is_default      = $is_default,
            category        = $category,
            download_limit  = $download_limit,
            max_connections = $max_connections,
            max_uploads     = $max_uploads,
            metadata        = $metadata,
            session_id      = $session_id,
            save_path       = $save_path,
            storage_mode    = $storage_mode,
            tags            = $tags,
            upload_limit    = $upload_limit
        WHERE
            id = $id
        )sql");

    stmt.Bind("$id",              preset.id);
    stmt.Bind("$name",            preset.name);
    stmt.Bind("$is_default",      preset.is_default ? 1 : 0);
    stmt.Bind("$category",        preset.category);
    stmt.Bind("$download_limit",  preset.download_limit);
    stmt.Bind("$max_connections", preset.max_connections);
    stmt.Bind("$max_uploads",     preset.max_uploads);
    stmt.Bind("$metadata",        metadata);
    stmt.Bind("$session_id",      preset.session_id);
    stmt.Bind("$save_path",       preset.save_path);
    stmt.Bind("$storage_mode",    preset.storage_mode);
    stmt.Bind("$tags",            tags);
    stmt.Bind("$upload_limit",    preset.upload_limit);
    stmt.Execute();
}
