local M = {}

function M.migrate(db)
    print("Creating 'addtorrentparams' table")

    db:exec([[
        CREATE TABLE addtorrentparams (
            id INTEGER PRIMARY KEY,
            info_hash_v1 TEXT,
            info_hash_v2 TEXT,
            name TEXT,
            queue_position INTEGER NOT NULL,
            resume_data_buf BLOB NOT NULL,
            save_path TEXT NOT NULL,
            CHECK (info_hash_v1 IS NOT NULL OR info_hash_v2 IS NOT NULL),
            UNIQUE (info_hash_v1),
            UNIQUE (info_hash_v2)
        );
    ]])

    print("Creating 'sessionparams' table")

    db:exec([[
        CREATE TABLE sessionparams (
            data BLOB NOT NULL,
            timestamp INTEGER NOT NULL DEFAULT (strftime('%s'))
        );
    ]])
end

return M
