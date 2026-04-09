local M = {}

function M.migrate(db)
    print("Creating 'sessionsettings' table")

    db:exec([[
        CREATE TABLE sessionsettings (
            id INTEGER PRIMARY KEY,
            key TEXT NOT NULL,
            value TEXT NOT NULL,
            UNIQUE (key)
        );
    ]])
end

return M
