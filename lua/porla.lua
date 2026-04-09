local config   = require "config"
local db       = require "db"
local sessions = require "sessions"
local sqlite3  = require "sqlite3"

function on_load()
    local http_server = uws.app()

    http_server:listen("127.0.0.1", 1337, function()
        print("http up")
    end)

    print("after")
    local cfg = config.get()
    local db_file = sqlite3.open(cfg.state_dir .. "/" .. "porla.sqlite")


    local ok, result = pcall(db.migrate, db_file)
    if not ok then
        print("migration error: " .. tostring(result))
        return nil
    end
    

    sessions.load_all(db_file)

end

function on_unload()
    print("destroying")
end
