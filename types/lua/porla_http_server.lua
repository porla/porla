---@meta porla_http_server

local http_server = {}

---@param path string
---@param callback fun(req: PoHttpServerRequest, res: PoHttpServerResponse)
function http_server.get(path, callback) end

---@param path string
---@param callback fun(req: PoHttpServerRequest, res: PoHttpServerResponse)
function http_server.post(path, callback) end

return http_server
