R"luastring"--(

local events  = require "events"
local signals = {}

return {
    added = function()
        return function(callback)
            local signal = events.on("torrent_added", function(torrent)
                callback(torrent)
            end)
    
            table.insert(signals, signal)
        end
    end,

    finished = function()
        return function(callback)
            local signal = events.on("torrent_finished", function(torrent)
                callback(torrent)
            end)

            table.insert(signals, signal)
        end
    end,

    moved = function()
        return function(callback)
            local signal = events.on("torrent_moved", function(torrent)
                callback(torrent)
            end)

            table.insert(signals, signal)
        end
    end
}

--)luastring"--"
