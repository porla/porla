R"luastring"--(

local torrents = require "torrents"
local signals = {}

return {
    added = function()
        return function(callback)
            local signal = torrents.on("added", function(torrent)
                callback(torrent)
            end)
    
            table.insert(signals, signal)
        end
    end,

    finished = function()
        return function(callback)
            local signal = torrents.on("finished", function(torrent)
                callback(torrent)
            end)

            table.insert(signals, signal)
        end
    end,

    moved = function()
        return function(callback)
            local signal = torrents.on("moved", function(torrent)
                callback(torrent)
            end)

            table.insert(signals, signal)
        end
    end
}

--)luastring"--"
