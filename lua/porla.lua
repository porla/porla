local sqlite3 = require("sqlite3")

print("loading porla")

local db = sqlite3.open("porla.sqlite")
local stmt = db:prepare("select id,name from sessions")

while (stmt:step() == sqlite3.ROW) do
    print("Loading session", stmt:get_value(1))

    local session = lt.session()
    session:set_alert_notify(function (msg)
        print("amaze amaze amaze", msg)
    end)
end

stmt:finalize()

-- hello
print("hello world")