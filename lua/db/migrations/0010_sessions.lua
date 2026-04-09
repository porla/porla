local config = require "config"

local M = {}

function M.migrate(db)
    print("Adding 'sessions' table")

    db:exec([[
        CREATE TABLE sessions (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL UNIQUE,
            is_default INTEGER NOT NULL DEFAULT 0,
            metadata BLOB NULL,
            params BLOB NULL,
            settings BLOB NOT NULL,
            timer_dht_stats INTEGER NOT NULL DEFAULT 5000,
            timer_save_state INTEGER NOT NULL DEFAULT 300000,
            timer_session_stats INTEGER NOT NULL DEFAULT 5000,
            timer_torrent_updates INTEGER NOT NULL DEFAULT 1000,
            CHECK (is_default IN (0, 1))
        );

        CREATE UNIQUE INDEX uq_sessions_default
        ON sessions(is_default)
        WHERE is_default = 1;
    ]])

    for k, v in pairs(config.sessions or {}) do
        print("Inserting session", k)
    end
end

return M
