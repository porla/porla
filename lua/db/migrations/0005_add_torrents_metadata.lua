local M = {}

function M.migrate(db)
    print("Creating 'torrentsmetadata' table")

    db:exec([[
        CREATE TABLE torrentsmetadata (
            id INTEGER PRIMARY KEY,
            info_hash_v1 TEXT,
            info_hash_v2 TEXT,
            key TEXT NOT NULL,
            value TEXT NOT NULL,
            CHECK (info_hash_v1 IS NOT NULL OR info_hash_v2 IS NOT NULL),
            UNIQUE (info_hash_v1, info_hash_v2, key)
        );
    ]])
end

return M
