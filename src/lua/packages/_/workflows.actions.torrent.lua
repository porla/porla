R"luastring"--(

local events   = require "events"
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
            local signal_connection = nil

            signal_connection = events.on("torrent_moved", function(torrent)
                if torrent.info_hash == ctx.torrent.info_hash then
                    signal_connection = nil
                    callback()
                end
            end)

            torrents.move(ctx.torrent, args)
        end
    end,

    pause = function(args)
        return function(ctx, callback)
            -- if the torrent is already paused, do nothing, just continue the workflow

            if ctx.torrent.is_paused then
                callback()
                return
            end

            local signal_connection = nil

            signal_connection = events.on("torrent_paused", function(torrent)
                print(torrent.info_hash, ctx.torrent.info_hash)
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
            local signal_connection = nil

            signal_connection = events.on("torrent_removed", function(info_hash)
                if info_hash == ctx.torrent.info_hash then
                    signal_connection = nil
                    callback()
                end
            end)

            torrents.remove(ctx.torrent)
        end
    end,

    resume = function(args)
        return function(ctx, callback)
            -- if the torrent is already resumed, do nothing, just continue the workflow

            if not ctx.torrent.is_paused then
                callback()
                return
            end

            local signal_connection = nil

            signal_connection = events.on("torrent_resumed", function(torrent)
                if torrent.info_hash == ctx.torrent.info_hash then
                    signal_connection = nil
                    callback()
                end
            end)

            torrents.resume(ctx.torrent)
        end
    end
}

--)luastring"--"
