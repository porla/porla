local M = {}

function M.migrate(db)
    print("Adding client_data column to addtorrentparams table")

    db:exec([[
        ALTER TABLE addtorrentparams ADD COLUMN client_data TEXT NULL;
    ]])

    print("Dropping torrentsmetadata table")

    db:exec([[
        DROP TABLE torrentsmetadata;
    ]])
end

return M
