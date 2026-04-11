local config              = require "config"
local db                  = require "db"
local http_server         = require "http.server"
local http_server_metrics = require "http.metrics"
local sessions            = require "sessions"
local sqlite3             = require "sqlite3"

function on_load()
    print("on_load")
    print("sleeping 1s")

    timer.sleep(1000)

    print("slept like baby")

    local res, err = libcurl.request({
        url = "https://httpbin.org/delay/5"
    })

    print("err", err)
    print("response", res.status)
end

function on_unload()
    print("un_unload")

    local res, err = libcurl.request({
        url = "https://httpbin.org/delay/3"
    })

    print("response", res.status)
end
