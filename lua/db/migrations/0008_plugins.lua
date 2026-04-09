local M = {}

function M.migrate(db)
    print("Creating 'plugins' table")

    db:exec([[
        CREATE TABLE plugins (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL UNIQUE,
            path TEXT NOT NULL UNIQUE,
            config TEXT,
            enabled INTEGER NOT NULL DEFAULT 0
        );
    ]])
end

return M
