R"luastring"--(
local sessions = require "sessions"
local timers   = require "timers"

local signals = {}

return function(interval)
    return function(callback)
        local signal = timers.new({
            interval = interval,
            callback = function()
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
