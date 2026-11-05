---@meta

---The files of a torrent as in its metadata, without renames (see `LtFilenames`). Indices are
---0-based; one out of range raises a `PoError` with `code = "invalid_params"`.
---@class LtFileStorage
local LtFileStorage = {}

---@param index integer
---@return string # the leaf name
function LtFileStorage:file_name(index) end

---@param index integer
---@return string # relative to the save path
function LtFileStorage:file_path(index) end

---@param index integer
---@return integer # bytes
function LtFileStorage:file_size(index) end
