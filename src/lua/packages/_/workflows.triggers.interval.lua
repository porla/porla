R"luastring"--(
local timers   = require "timers"
local torrents = require "torrents"

local signals = {}

return function(interval)
    return function(callback)
        local signal = timers.new({
            interval = interval,
            callback = function()
                for _, torrent in pairs(torrents.list()) do
                    callback(torrent)
                end
            end
        })

        table.insert(signals, signal)
    end
end

--)luastring"--"
