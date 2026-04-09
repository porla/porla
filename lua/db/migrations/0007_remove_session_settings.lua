local M = {}

function M.migrate(db)
    print("Removing 'sessionsettings' table")

    db:exec([[
        DROP TABLE sessionsettings;
    ]])
end

return M
