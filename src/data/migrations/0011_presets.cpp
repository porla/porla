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
        R"sql(
        CREATE TABLE presets (
            id              INTEGER PRIMARY KEY,
            name            TEXT NOT NULL UNIQUE,
            is_default      INTEGER NOT NULL DEFAULT 0,
            category        TEXT,
            download_limit  INTEGER,
            max_connections INTEGER,
            max_uploads     INTEGER,
            metadata        TEXT,
            session_id      INTEGER REFERENCES sessions(id),
            save_path       TEXT,
            storage_mode    TEXT,
            tags            TEXT,
            upload_limit    INTEGER
        );

        CREATE UNIQUE INDEX uq_presets_default
        ON presets(is_default)
        WHERE is_default = 1;
        )sql",
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
            R"sql(
            INSERT INTO presets (
                name,
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
            )
            VALUES (
                $name,
                $category,
                $download_limit,
                $max_connections,
                $max_uploads,
                $metadata,
                NULL,
                $save_path,
                $storage_mode,
                $tags,
                $upload_limit
            );
            )sql");

        stmt.Bind("$name",            name);
        stmt.Bind("$category",        preset.category);
        stmt.Bind("$download_limit",  preset.download_limit);
        stmt.Bind("$max_connections", preset.max_connections);
        stmt.Bind("$max_uploads",     preset.max_uploads);
        stmt.Bind("$metadata",        metadata.dump());
        stmt.Bind("$save_path",       preset.save_path);
        stmt.Bind("$storage_mode",    storage_mode);
        stmt.Bind("$tags",            json(preset.tags).dump());
        stmt.Bind("$upload_limit",    preset.upload_limit);
        stmt.Execute();
    }

    res = sqlite3_exec(
        db,
        "UPDATE presets SET is_default = 1 WHERE name = 'default'",
        nullptr,
        nullptr,
        nullptr);

    return res;
}
