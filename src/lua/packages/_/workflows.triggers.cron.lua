R"luastring"--(
local cron     = require "cron"
local sessions = require "sessions"

local signals = {}

return function(expression)
    return function(callback)
        local signal = cron.schedule({
            expression = expression,
            callback   = function()
                for session in sessions.list() do
                    for torrent in session:torrents() do
                        callback(torrent)
                    end
                end
            end
        })

        table.insert(signals, signal)
    end
end
--)luastring"--"
