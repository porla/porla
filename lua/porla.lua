local config              = require "config"
local db                  = require "db"
local http_server         = require "http.server"
local http_server_metrics = require "http.metrics"
local sessions            = require "sessions"
local sqlite3             = require "sqlite3"

function on_load()
    local archive, error = zip.zip_t.open("api.zip", zip.ZIP_RDONLY)

    if error then
        print(error)
        return
    end

    log.info("archive loaded")

    for i = 0, archive:get_num_entries(zip.ZIP_FL_UNCHANGED) - 1 do
        local stat, error = archive:stat_index(i)

        if error then
            print(error)
            break
        end

        if stat.size > 0 then
            local file = archive:fopen_index(i, zip.ZIP_FL_UNCHANGED)
            local buffer, error = file:fread(stat.size)

            if error then
                print("error reading file")
            else
                print(stat.name, "(" .. #buffer .. " bytes)")
            end

            file:fclose()
            file = nil
        end
    end

    print("closing archive")

    archive:close()
    archive = nil

    print("archive unloaded")

    http_server:get("/metrics", http_server_metrics)

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
