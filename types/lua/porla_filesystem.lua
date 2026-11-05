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

---Creates a hardlink `dst` to the file `src`, and any missing parent directories of `dst`. Never
---replaces an existing `dst`. Both paths must be on the same filesystem. Relative paths resolve
---against Porla's working directory.
---@param src string
---@param dst string
---@return boolean? linked # true
---@return PoError? err # `io_error` with the OS `category`/`value`, e.g. file exists, or cross-device link
function fs.link(src, dst) end

---@class PoFilesystemSpace
---@field available integer # bytes free for Porla's user (what a download can use)
---@field capacity integer # size of the filesystem in bytes
---@field free integer # bytes free, including space reserved for root

---Space on the filesystem that holds `path`. Relative paths resolve against Porla's working directory.
---@param path string
---@return PoFilesystemSpace?
---@return PoError? err # `io_error` with the OS `category`/`value`, e.g. no such file or directory
function fs.space(path) end

return fs
