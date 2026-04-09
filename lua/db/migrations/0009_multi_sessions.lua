local M = {}

function M.migrate(db)
    print("Adding 'session_id' column to table 'addtorrentparams'")

    db:exec([[
        ALTER TABLE addtorrentparams ADD COLUMN session_id TEXT NOT NULL DEFAULT 'default';
    ]])
end

return M
