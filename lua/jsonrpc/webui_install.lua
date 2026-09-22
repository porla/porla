local jsonrpc = require("porla_jsonrpc")
local webui   = require("webui")

return function()
    jsonrpc.register("webui.install", function(params, res)
        if type(params.version) ~= "string" then
            return res:error(-1, "Missing or invalid version")
        end

        local version = string.format("tags/%s", params.version)

        webui.install(version, function(err, file)
            if err then
                return res:error(-2, string.format("Failed to install web UI: %s", err))
            end

            if not webui.load_archive(file) then
                return res:error(-3, "Failed to load web UI archive")
            end

            webui.set_current(file)

            res:ok({})
        end)
    end)
end
