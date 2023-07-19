R"luastring"--(

local events   = require "events"
local log      = require "log"
local torrents = require "torrents"

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
        return function(ctx, callback)
            local tracker_reply_connection = nil
            local tracker_error_connection = nil

            local current_tries = 0
            local interval      = 2
            local max_tries     = 5
            local tracker_index = 0

            if args and type(args.interval) == "number" then
                interval = args.interval
            end

            if args and type(args.max_tries) == "number" then
                max_tries = args.max_tries
            end

            if args and type(args.tracker_index) == "number" then
                tracker_index = args.tracker_index
            end

            tracker_reply_connection = events.on("torrent_tracker_reply", function(torrent)
                if torrent.info_hash ~= ctx.torrent.info_hash then
                    return
                end

                log.info("Torrent successfully reannounced")

                tracker_reply_connection:disconnect()
                tracker_reply_connection = nil

                tracker_error_connection:disconnect()
                tracker_error_connection = nil

                callback()
            end)

            tracker_error_connection = events.on("torrent_tracker_error", function(ev)
                if ev.torrent.info_hash ~= ctx.torrent.info_hash then
                    return
                end

                if ev.error ~= nil and ev.error.value == torrents.errors.tracker_failure then
                    local match_failures = {
                        "not exist",
                        "not found",
                        "not registered",
                        "unregistered",
                        "not authorized"
                    }

                    local found_matching_failure = false

                    for _, message in ipairs(match_failures) do
                        local i, j = string.find(ev.failure_reason, message)

                        if i and i > 0 then
                            current_tries          = current_tries + 1
                            found_matching_failure = true

                            if current_tries > max_tries then
                                log.warning "Max tries reached for reannounce"

                                tracker_reply_connection:disconnect()
                                tracker_reply_connection = nil

                                tracker_error_connection:disconnect()
                                tracker_error_connection = nil

                                callback()

                                return
                            end

                            log.info(string.format("Reannouncing torrent %s (attempt %d of %d)", ctx.torrent.name, current_tries, max_tries))

                            torrents.reannounce(ctx.torrent, {
                                seconds       = interval,
                                tracker_index = tracker_index
                            })
                        end
                    end

                    if not found_matching_failure then
                        log.warning("No matching failure reason found - skipping reannounce logic")

                        tracker_reply_connection:disconnect()
                        tracker_reply_connection = nil

                        tracker_error_connection:disconnect()
                        tracker_error_connection = nil

                        callback()
                    end
                end
            end)

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
