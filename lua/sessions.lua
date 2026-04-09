local sqlite3 = require("sqlite3")

local M = {}

M._sessions = {}

function M.load_all(db)
    local stmt = db:prepare("select id,name from sessions")

    while (stmt:step() == sqlite3.ROW) do
        print("Loading session", stmt:get_value(1))

        local alert_mask = lt.setting_by_name("alert_mask")
        local settings = lt.default_settings()

        -- settings:set_int(alert_mask, lt.alert.all_categories)

        local session = lt.session(settings)
        session:add_extension("smart_ban")
        session:add_extension("ut_metadata")
        session:add_extension("ut_pex")

        session:set_alert_notify(function (msg)
            print("amaze amaze amaze", msg)
        end)

        table.insert(M._sessions, session)
    end

    stmt:finalize()
end

return M
