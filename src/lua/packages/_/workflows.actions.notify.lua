R"luastring"--(
local http = require "http.client"
local json = require "json"

local function resolve(arg, ctx)
    if type(arg) == "function" then
        return arg(ctx)
    end

    return arg
end

return {
    discord = function(args)
        return function(ctx, callback)
            local body = json.encode({
                content = resolve(args.message, ctx)
            })

            http.request({
                url      = resolve(args.url, ctx),
                method   = "POST",
                headers  = {
                    ["Content-Type"] = "application/json"
                },
                body     = body,
                callback = function(status_code, content)
                    callback()
                end
            })
        end
    end,

    ntfy = function(args)
        return function(ctx, callback)
            http.request({
                url      = "https://ntfy.sh/"..resolve(args.topic, ctx),
                method   = "POST",
                body     = resolve(args.message, ctx),
                callback = function(status_code, content)
                    callback()
                end
            })
        end
    end
}
--)luastring"--"
