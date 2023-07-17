R"luastring"--(
local timers = require "timers"

return function(args)
    return function(ctx, callback)
        local timer = nil

        timer = timers.new({
            interval = args,
            callback = function()
                timer:cancel()
                timer = nil

                callback()
            end
        })
    end
end
--)luastring"--"
