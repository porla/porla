R"luastring"--(
local cron     = require "cron"
local torrents = require "torrents"

local signals = {}

return function(expression)
    return function(callback)
        local signal = cron.schedule({
            expression = expression,
            callback   = function()
                for _, torrent in pairs(torrents.list()) do
                    callback(torrent)
                end
            end
        })

        table.insert(signals, signal)
    end
end
--)luastring"--"
