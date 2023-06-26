R"luastring"--(
local process = require "process"

return function(args)
    return function(ctx, callback)
        process.launch({
            file = args.file,
            args = args.args,
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
