R"luastring"--(
local process = require "process"

return function(args)
    return function(ctx, callback)
        local actual_args = {}

        for i, item in ipairs(args.args) do
            if type(item) == "function" then
                actual_args[i] = item(ctx)
            else
                actual_args[i] = item
            end
        end

        process.launch({
            file = args.file,
            args = actual_args,
            done = function(exit_code, stdout, stderr)
                callback({
                    exit_code = exit_code,
                    std_out   = stdout,
                    std_err   = stderr
                })
            end
        })
    end
end
--)luastring"--"
