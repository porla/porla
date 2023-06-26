R"luastring"--(
local cron     = require "cron"
local torrents = require "torrents"

return function(expression)
    return function(callback)
        cron.schedule({
            expression = expression,
            callback   = function()
                for _, torrent in pairs(torrents.list()) do
                    callback(torrent)
                end
            end
        })
    end
end
--)luastring"--"
