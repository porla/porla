#include "../migrations.hpp"

#include <boost/log/trivial.hpp>

int porla::Data::Migrations::AddAutoIncrementIds(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Making ID columns of plugins, presets and sessions AUTOINCREMENT";

    return sqlite3_exec(
        db,
        R"sql(
        -- create new sessions table
        CREATE TABLE sessions_new (
            id                    INTEGER PRIMARY KEY AUTOINCREMENT,
            name                  TEXT NOT NULL UNIQUE,
            is_default            INTEGER NOT NULL DEFAULT 0,
            metadata              BLOB NULL,
            params                BLOB NULL,
            timer_dht_stats       INTEGER NOT NULL DEFAULT 5000,
            timer_save_state      INTEGER NOT NULL DEFAULT 300000,
            timer_session_stats   INTEGER NOT NULL DEFAULT 5000,
            timer_torrent_updates INTEGER NOT NULL DEFAULT 1000,
            CHECK (is_default IN (0, 1))
        );

        -- transfer sessions
        INSERT INTO sessions_new (
            id,
            name,
            is_default,
            metadata,
            params,
            timer_dht_stats,
            timer_save_state,
            timer_session_stats,
            timer_torrent_updates
        )
        SELECT
            id,
            name,
            is_default,
            metadata,
            params,
            timer_dht_stats,
            timer_save_state,
            timer_session_stats,
            timer_torrent_updates
        FROM sessions;

        -- drop old sessions and rename new one
        DROP TABLE sessions;
        ALTER TABLE sessions_new RENAME TO sessions;

        -- create the index making us have only one default session
        CREATE UNIQUE INDEX uq_sessions_default ON sessions(is_default) WHERE is_default = 1;

        -- presets
        CREATE TABLE presets_new (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
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
            upload_limit    INTEGER,
            flags           INTEGER,
            flags_mask      INTEGER
        );

        INSERT INTO presets_new (
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
            upload_limit,
            flags,
            flags_mask
        )
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
            upload_limit,
            flags,
            flags_mask
        FROM presets;

        DROP TABLE presets;
        ALTER TABLE presets_new RENAME TO presets;

        CREATE UNIQUE INDEX uq_presets_default ON presets(is_default) WHERE is_default = 1;

        -- plugins
        CREATE TABLE plugins_new (
            id       INTEGER PRIMARY KEY AUTOINCREMENT,
            path     TEXT NOT NULL,
            config   TEXT,
            metadata TEXT
        );

        INSERT INTO plugins_new (id, path, config, metadata)
        SELECT id, path, config, metadata FROM plugins;

        DROP TABLE plugins;
        ALTER TABLE plugins_new RENAME TO plugins;

        -- seed the session sequence above every session id still referenced in the database, so an
        -- id that was deleted but is still used by torrent rows or presets is never handed out again
        DELETE FROM sqlite_sequence WHERE name = 'sessions';
        INSERT INTO sqlite_sequence (name, seq)
        SELECT 'sessions', MAX(
            COALESCE((SELECT MAX(id) FROM sessions), 0),
            COALESCE((SELECT MAX(session_id) FROM addtorrentparams), 0),
            COALESCE((SELECT MAX(session_id) FROM presets), 0));

        -- clean up references left by earlier removals (after seeding, which reads them)
        UPDATE presets SET session_id = NULL WHERE session_id IS NOT NULL
            AND session_id NOT IN (SELECT id FROM sessions);
        )sql",
        nullptr,
        nullptr,
        nullptr);
}
