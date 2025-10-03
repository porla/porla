#include "0011_presets.hpp"

#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

#include "../../config.hpp"
#include "../statement.hpp"

using json = nlohmann::json;
using porla::Data::Migrations::Presets;

namespace lt = libtorrent;

int Presets::Migrate(sqlite3 *db, const std::unique_ptr<porla::Config> &cfg)
{
    BOOST_LOG_TRIVIAL(info) << "Adding 'presets' table";

    int res = sqlite3_exec(
        db,
        "CREATE TABLE presets ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL UNIQUE,"
        "category TEXT,"
        "download_limit INTEGER,"
        "max_connections INTEGER,"
        "max_uploads INTEGER,"
        "metadata TEXT,"
        "session_id INTEGER,"
        "save_path TEXT,"
        "storage_mode TEXT,"
        "tags TEXT,"
        "upload_limit INTEGER"
        ");",
        nullptr,
        nullptr,
        nullptr);

    if (res != SQLITE_OK)
    {
        return res;
    }

    for (const auto& [name, preset] : cfg->presets)
    {
        BOOST_LOG_TRIVIAL(info) << "Inserting preset " << name;

        std::optional<std::string> storage_mode = preset.storage_mode.has_value()
            ? preset.storage_mode.value() == lt::storage_mode_allocate ? "allocate" : "sparse"
            : std::optional<std::string>();

        json metadata;

        if (preset.dollar_hidden.has_value())
        {
            metadata["$hidden"] = preset.dollar_hidden.value();
        }

        auto stmt = Statement::Prepare(
            db,
            "INSERT INTO presets (name, category, download_limit, max_connections, max_uploads, metadata, session_id, save_path, storage_mode, tags, upload_limit) "
            "VALUES ($1, $2, $3, $4, $5, $6, NULL, $7, $8, $9, $10)");
        stmt.Bind(1, std::string_view(name));
        stmt.Bind(2, preset.category);
        stmt.Bind(3, preset.download_limit);
        stmt.Bind(4, preset.max_connections);
        stmt.Bind(5, preset.max_connections);
        stmt.Bind(6, std::string_view(metadata.dump()));
        stmt.Bind(7, preset.save_path);
        stmt.Bind(8, storage_mode);
        stmt.Bind(9, std::string_view(json(preset.tags).dump()));
        stmt.Bind(10, preset.upload_limit);
        stmt.Execute();
    }

    return res;
}
