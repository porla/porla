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
        local resolved_headers = {}

        if args.headers ~= nil then
            for key, val in pairs(args.headers) do
                resolved_headers[key] = resolve(val, ctx)
            end
        end

        http.request({
            body     = resolve(args.body, ctx),
            method   = resolve(args.method, ctx),
            url      = resolve(args.url, ctx),
            headers  = resolved_headers,
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
