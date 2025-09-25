#include "presets.hpp"

#include "../statement.hpp"

using porla::Data::Models::Presets;

const std::string PresetSelectPrefix = "SELECT id,name,category,download_limit,max_connections,max_uploads,metadata,session,save_path,storage_mode,tags,upload_limit FROM presets";

static Presets::Preset LoadFromRow(const porla::Data::Statement::IRow &row)
{
    return Presets::Preset{
        .id = row.GetInt32(0),
        .name = row.GetStdString(1),
        .category = row.GetStdString(2),
        .download_limit = row.GetOptionalInt32(3),
        .max_connections = row.GetOptionalInt32(4),
        .max_uploads = row.GetOptionalInt32(5),
        .metadata = {},
        .session = row.GetStdString(7),
        .save_path = row.GetStdString(8),
        .storage_mode = row.GetStdString(9),
        .tags = {},
        .upload_limit = row.GetOptionalInt32(11)
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

    Statement::Prepare(db, PresetSelectPrefix + " WHERE id = $1")
        .Bind(1, id)
        .Step(
            [&preset](auto const &row)
            {
                preset = LoadFromRow(row);
                return SQLITE_OK;
            });

    return preset;
}

std::optional<Presets::Preset> Presets::GetByName(sqlite3 *db, const std::string &name)
{
    std::optional<Preset> preset;

    Statement::Prepare(db, PresetSelectPrefix + " WHERE name = $1")
        .Bind(1, std::string_view(name))
        .Step(
            [&preset](auto const &row)
            {
                preset = LoadFromRow(row);
                return SQLITE_OK;
            });

    return preset;
}

void Presets::Insert(sqlite3 *db, const Presets::Preset &preset)
{
}

void Presets::Update(sqlite3 *db, const Presets::Preset &preset)
{
    auto stmt = Statement::Prepare(
        db,
        "UPDATE presets SET\n"
        "name = $1,"
        "category = $2,"
        "download_limit = $3,"
        "max_connections = $4,"
        "max_uploads = $5,"
        "metadata = $6,"
        "session = $7,"
        "save_path = $8,"
        "storage_mode = $9,"
        "tags = $10,"
        "upload_limit = $11\n"
        "WHERE id = $12");

    stmt.Bind(1, std::string_view(preset.name));
    stmt.Bind(2, preset.category);
    stmt.Bind(3, preset.download_limit);
    stmt.Bind(4, preset.max_connections);
    stmt.Bind(5, preset.max_uploads);
    stmt.Bind(6, preset.metadata ? std::optional<std::string_view>(preset.metadata->dump()) : std::nullopt);
    stmt.Bind(7, preset.session);
    stmt.Bind(8, preset.save_path);
    stmt.Bind(9, preset.storage_mode);
    stmt.Bind(10, std::string_view(nlohmann::json(preset.tags).dump()));
    stmt.Bind(11, preset.upload_limit);
    stmt.Bind(12, preset.id);
    stmt.Execute();
}
