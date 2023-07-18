R"luastring"--(
local http = require "http.client"

local function resolve(arg, ctx)
    if type(arg) == "function" then
        return arg(ctx)
    end

    return arg
end

return function(args)
    return function(ctx, callback)
        http.request({
            body     = args.body,
            method   = resolve(args.method, ctx),
            url      = resolve(args.url, ctx),
            callback = function(status_code, content)
                callback({
                    content     = content,
                    status_code = status_code
                })
            end
        })
    end
end
--)luastring"--"
