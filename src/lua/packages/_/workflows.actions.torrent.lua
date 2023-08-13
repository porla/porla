R"luastring"--(

local events   = require "events"
local log      = require "log"
local sessions = require "sessions"
local timers   = require "timers"

local function resolve(arg, ctx)
    if type(arg) == "function" then
        return arg(ctx)
    end

    return arg
end

return {
    move = function(args)
        return function(ctx, callback)
            local signal_connection = nil

            signal_connection = events.on("torrent_moved", function(torrent)
                if torrent:info_hash() == ctx.torrent:info_hash() then
                    signal_connection = nil
                    callback()
                end
            end)

            ctx.torrent:move_storage(resolve(args.path, ctx))
        end
    end,

    pause = function(args)
        return function(ctx, callback)
            -- if the torrent is already paused, do nothing, just continue the workflow

            if ctx.torrent:flags().paused then
                callback()
                return
            end

            local signal_connection = nil

            signal_connection = events.on("torrent_paused", function(torrent)
                if torrent:info_hash() == ctx.torrent:info_hash() then
                    signal_connection = nil
                    callback()
                end
            end)

            ctx.torrent:pause()
        end
    end,

    remove = function(args)
        return function(ctx, callback)
            local removing_info_hash = ctx.torrent:info_hash()
            local signal_connection  = nil

            signal_connection = events.on("torrent_removed", function(info_hash)
                if info_hash == removing_info_hash then
                    signal_connection = nil
                    callback()
                end
            end)

            for session in sessions.list() do
                if session:find_torrent(ctx.torrent:info_hash()) then
                    session:remove_torrent(ctx.torrent, args)
                end
            end
        end
    end,

    resume = function(args)
        return function(ctx, callback)
            -- if the torrent is already resumed, do nothing, just continue the workflow

            if not ctx.torrent:flags().paused then
                callback()
                return
            end

            local signal_connection = nil

            signal_connection = events.on("torrent_resumed", function(torrent)
                if torrent:info_hash() == ctx.torrent:info_hash() then
                    signal_connection = nil
                    callback()
                end
            end)

            ctx.torrent:resume()
        end
    end
}

--)luastring"--"
