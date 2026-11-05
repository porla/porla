---@meta porla_http_server

local http_server = {}

---Routes are served on Porla's own HTTP server and removed when the plugin unloads. `path` takes uWebSockets
---patterns (`*`, `:name`), but matched parameters are not exposed. The handler runs once the whole body is
---received; if it raises, Porla answers 500, or closes the connection if the response was already started.
---@alias PoHttpServerHandler fun(req: PoHttpServerRequest, res: PoHttpServerResponse)

---@param path string
---@param callback PoHttpServerHandler
function http_server.get(path, callback) end

---@param path string
---@param callback PoHttpServerHandler
function http_server.post(path, callback) end

---@param path string
---@param callback PoHttpServerHandler
function http_server.put(path, callback) end

---@param path string
---@param callback PoHttpServerHandler
function http_server.delete(path, callback) end

---Matches every method.
---@param path string
---@param callback PoHttpServerHandler
function http_server.any(path, callback) end

return http_server
