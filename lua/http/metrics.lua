local M = function(res, req)
    local url = req:getUrl()
    res:writeHeader("Content-Type", "text/plain")
    res["end"](res, "Hello from " .. url)
end

return M
