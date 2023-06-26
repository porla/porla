R"luastring"--(

local torrents = require "torrents"

return {
    flags = {
        set = function(args)
            return function(ctx, callback)
                torrents.setFlags(ctx.torrent, args)
                callback()
            end
        end,

        unset = function(args)
            return function(ctx, callback)
                torrents.unsetFlags(ctx.torrent, args)
                callback()
            end
        end,
    },

    move = function(args)
        return function(ctx, callback)
            local moved_signal = torrents.on("moved", function(torrent)
                if torrent.info_hash == ctx.torrent.info_hash then
                    moved_signal = nil
                    callback()
                end
            end)
        end
    end,

    pause = function(args)
        return function(ctx, callback)
            local signal_connection = torrents.on("paused", function(torrent)
                if torrent.info_hash == ctx.torrent.info_hash then
                    signal_connection = nil
                    callback()
                end
            end)

            torrents.pause(ctx.torrent)
        end
    end,

    reannounce = function(args)
    end,

    remove = function(args)
        return function(ctx, callback)
            local signal_connection = torrents.on("removed", function(info_hash)
                if info_hash == ctx.torrent.info_hash then
                    signal_connection = nil
                    callback()
                end
            end)

            torrents.remove(ctx.torrent)
        end
    end,

    resume = function(args)
        local signal_connection = torrents.on("resumed", function(torrent)
            if torrent.info_hash == ctx.torrent.info_hash then
                signal_connection = nil
                callback()
            end
        end)

        torrents.resume(ctx.torrent)
    end
}

--)luastring"--"
