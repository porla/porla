R"luastring"--(
local log = require "log"

return function(args)
    return function(ctx, callback)
        if type(args) == "function" then
            log.info(args(ctx))
        else
            log.info(args)
        end

        callback()
    end
end
--)luastring"--"
