R"luastring"--(

local events   = require "events"
local log      = require "log"
local timers   = require "timers"
local torrents = require "torrents"

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
                if torrent.info_hash == ctx.torrent.info_hash then
                    signal_connection = nil
                    callback()
                end
            end)

            torrents.move(ctx.torrent, {
                path = resolve(args.path, ctx)
            })
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
                if torrent.info_hash == ctx.torrent.info_hash then
                    signal_connection = nil
                    callback()
                end
            end)

            torrents.pause(ctx.torrent)
        end
    end,

    reannounce = function(args)
        return function(ctx, callback)
            local current_tries    = 0
            local interval         = 2
            local max_tries        = 5
            local tracker_index    = 0
            local reannounce_timer = nil

            if args and type(args.interval) == "number" then
                interval = args.interval
            end

            if args and type(args.max_tries) == "number" then
                max_tries = args.max_tries
            end

            if args and type(args.tracker_index) == "number" then
                tracker_index = args.tracker_index
            end

            local peers = torrents.peers.list(ctx.torrent)

            if #(peers) > 0 then
                log.info(string.format("Torrent has %d peer(s) - skipping reannounce", #(peers)))
                return callback()
            end

            reannounce_timer = timers.new({
                interval = interval * 1000,
                callback = function()
                    local peers = torrents.peers.list(ctx.torrent)

                    if #(peers) > 0 then
                        log.info("Torrent now has peers - finishing")

                        reannounce_timer:cancel()
                        reannounce_timer = nil

                        return callback()
                    end

                    current_tries = current_tries + 1

                    if current_tries >= max_tries then
                        log.warning(string.format("Max reannounce tries reached for %s", ctx.torrent.name))

                        reannounce_timer:cancel()
                        reannounce_timer = nil

                        return callback()
                    end

                    local match_failures = {
                        "not exist",
                        "not found",
                        "not registered",
                        "unregistered",
                        "not authorized"
                    }

                    local trackers = torrents.trackers.list(ctx.torrent)
                    local found_matching_failure = false

                    for _, tracker in ipairs(trackers) do
                        for _, endpoint in ipairs(tracker.endpoints) do
                            for _, aih in ipairs(endpoint.info_hashes) do
                                for _, message in ipairs(match_failures) do
                                    local i, j = string.find(aih.message, message)

                                    if i == nil or j == nil then
                                        goto next_message
                                    end

                                    found_matching_failure = true

                                    ::next_message::
                                end
                            end
                        end
                    end

                    if found_matching_failure then
                        log.info(string.format("Sending reannounce attempt %d of %d for %s", current_tries + 1, max_tries, ctx.torrent.name))

                        torrents.reannounce(ctx.torrent, {
                            seconds       = 0,
                            tracker_index = tracker_index
                        })

                        return
                    end

                    log.warning("No tracker matches any known failure - skipping reannounce")

                    reannounce_timer:cancel()
                    reannounce_timer = nil

                    return callback()
                end
            })

            log.info(string.format("Sending reannounce attempt %d of %d for %s", current_tries + 1, max_tries, ctx.torrent.name))

            torrents.reannounce(ctx.torrent, {
                seconds       = 0,
                tracker_index = tracker_index
            })
        end
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

            torrents.remove(ctx.torrent, args)
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
