---@meta

---@class LtFilenames
---@overload fun(storage: LtFileStorage, renamed: LtRenamedFiles): LtFilenames
LtFilenames = {}

---@param index integer
---@param save_path string?
---@return string
function LtFilenames:file_path(index, save_path) end

---@return integer
function LtFilenames:num_files() end

---@return integer
function LtFilenames:num_pieces() end
