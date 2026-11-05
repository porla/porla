---@meta porla_filesystem

local fs = {}

---Creates missing parents too. Relative paths resolve against Porla's working directory.
---@param path string
---@return boolean? created # false if the directory already existed
---@return PoError? err # `io_error` with the OS `category`/`value`
function fs.create_directories(path) end

---Relative paths resolve against Porla's working directory.
---@param path string
---@return boolean?
---@return PoError? err # `io_error` with the OS `category`/`value`, e.g. permission denied
function fs.exists(path) end

return fs
