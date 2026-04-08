local sqlite3 = require("sqlite3")

print("loading porla")

local db = sqlite3.open("porla.sqlite")
local stmt = db:prepare("select id,name from sessions")

while (stmt:step() == sqlite3.ROW) do
    print(stmt:get_value(1))
end

stmt:finalize()

-- hello
print("hello world")