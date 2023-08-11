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
    end,

    paused = function()
        return function(callback)
            local signal = events.on("torrent_paused", function(torrent)
                callback(torrent)
            end)

            table.insert(signals, signal)
        end
    end,

    removed = function()
        return function(callback)
            local signal = events.on("torrent_removed", function(info_hash)
                callback({ info_hash = info_hash })
            end)

            table.insert(signals, signal)
        end
    end,

    resumed = function()
        return function(callback)
            local signal = events.on("torrent_resumed", function(torrent)
                callback(torrent)
            end)

            table.insert(signals, signal)
        end
    end
}

--)luastring"--"
