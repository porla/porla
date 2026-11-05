---@meta porla_filesystem

local fs = {}

---@param path string
---@return boolean?
---@return PoError? err # `io_error` with the OS `category`/`value`
function fs.create_directories(path) end

---@param path string
---@return boolean?
---@return PoError? err # `io_error` with the OS `category`/`value`
function fs.exists(path) end

return fs
