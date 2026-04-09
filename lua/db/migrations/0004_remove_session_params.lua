local M = {}

function M.migrate(db)
    print("Removing 'sessionparams' table")

    db:exec([[
        DROP TABLE sessionparams;
    ]])
end

return M
