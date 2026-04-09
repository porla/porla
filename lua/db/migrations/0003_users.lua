local M = {}

function M.migrate(db)
    print("Creating 'users' table")

    db:exec([[
        CREATE TABLE users (
            id INTEGER PRIMARY KEY,
            username TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL
        );
    ]])
end

return M
