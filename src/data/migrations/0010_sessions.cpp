#include "0010_sessions.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/settings_pack.hpp>

#include "../../config.hpp"
#include "../statement.hpp"

using porla::Data::Migrations::Sessions;

namespace lt = libtorrent;

int Sessions::Migrate(sqlite3 *db, const std::unique_ptr<porla::Config> &cfg)
{
    BOOST_LOG_TRIVIAL(info) << "Adding 'sessions' table";

    int res = sqlite3_exec(
        db,
        R"sql(
        CREATE TABLE sessions (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL UNIQUE,
            is_default INTEGER NOT NULL DEFAULT 0,
            metadata BLOB NULL,
            params BLOB NULL,
            timer_dht_stats INTEGER NOT NULL DEFAULT 5000,
            timer_save_state INTEGER NOT NULL DEFAULT 300000,
            timer_session_stats INTEGER NOT NULL DEFAULT 5000,
            timer_torrent_updates INTEGER NOT NULL DEFAULT 1000,
            CHECK (is_default IN (0, 1))
        );

        CREATE UNIQUE INDEX uq_sessions_default
        ON sessions(is_default)
        WHERE is_default = 1;
        )sql",
        nullptr,
        nullptr,
        nullptr);

    if (res != SQLITE_OK)
    {
        return res;
    }

    for (const auto& [name, settings] : cfg->sessions)
    {
        BOOST_LOG_TRIVIAL(info) << "Inserting session " << name;
    
        lt::session_params params;
        params.settings = settings;

        std::vector params_buffer = lt::write_session_params_buf(params);

        auto stmt = Statement::Prepare(db, "INSERT INTO sessions (name, params) VALUES ($name, $params);");
        stmt.Bind("$name",   name);
        stmt.Bind("$params", params_buffer);
        stmt.Execute();
    }

    res = sqlite3_exec(
        db,
        "UPDATE sessions SET is_default = 1 WHERE name = 'default'",
        nullptr,
        nullptr,
        nullptr);

    res = sqlite3_exec(
        db,
        R"sql(
        -- rename addtorrentparams table since we want to create a new one that
        -- has a hard reference to session_id
        ALTER TABLE addtorrentparams RENAME TO addtorrentparams_prev;

        CREATE TABLE addtorrentparams (
            id INTEGER PRIMARY KEY,
            session_id INTEGER NOT NULL REFERENCES sessions(id),
            info_hash_v1 TEXT,
            info_hash_v2 TEXT,
            name TEXT,
            queue_position INTEGER NOT NULL,
            resume_data_buf BLOB NOT NULL,
            save_path TEXT NOT NULL,
            client_data TEXT NULL,
            CHECK (info_hash_v1 IS NOT NULL OR info_hash_v2 IS NOT NULL),
            UNIQUE (session_id, info_hash_v1),
            UNIQUE (session_id, info_hash_v2)
        );

        -- move all torrents from old addtorrentparams to new
        INSERT INTO addtorrentparams (
            session_id,
            info_hash_v1,
            info_hash_v2,
            name,
            queue_position,
            resume_data_buf,
            save_path,
            client_data
        )
        SELECT
            (SELECT id FROM sessions WHERE name = prev.session_id),
            prev.info_hash_v1,
            prev.info_hash_v2,
            prev.name,
            prev.queue_position,
            prev.resume_data_buf,
            save_path,
            client_data
        FROM addtorrentparams_prev prev;

        -- delete from prev table
        DELETE FROM addtorrentparams_prev
        WHERE EXISTS (
            SELECT 1
            FROM addtorrentparams curr
            JOIN sessions s ON s.id = curr.session_id
            WHERE s.name = addtorrentparams_prev.session_id
                AND (
                    (addtorrentparams_prev.info_hash_v1 = curr.info_hash_v1
                    AND addtorrentparams_prev.info_hash_v2 IS NULL
                    AND curr.info_hash_v2 IS NULL)

                    OR

                    (addtorrentparams_prev.info_hash_v1 IS NULL
                    AND curr.info_hash_v1 IS NULL
                    AND addtorrentparams_prev.info_hash_v2 = curr.info_hash_v2)

                    OR

                    (addtorrentparams_prev.info_hash_v1 = curr.info_hash_v1
                    AND addtorrentparams_prev.info_hash_v2 = curr.info_hash_v2)
                )
        )
        )sql",
        nullptr,
        nullptr,
        nullptr);

    return res;
}