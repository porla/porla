#include "../migrations.hpp"

#include <boost/log/trivial.hpp>

int porla::Data::Migrations::Presets(sqlite3 *db)
{
    BOOST_LOG_TRIVIAL(info) << "Adding 'presets' table";

    return sqlite3_exec(
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
}
