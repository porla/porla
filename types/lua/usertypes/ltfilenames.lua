---@meta

---File names with renames applied, e.g.
---`LtFilenames(status.torrent_file:layout(), status.renamed_files)`. Indices are 0-based; one
---out of range raises a `PoError` with `code = "invalid_params"`.
---@class LtFilenames
---@overload fun(storage: LtFileStorage, renamed: LtRenamedFiles): LtFilenames
LtFilenames = {}

---@param index integer
---@return boolean # true when the file was renamed to an absolute path
function LtFilenames:file_absolute_path(index) end

---Relative to the save path, or under `save_path` when given; an absolute rename ignores `save_path`.
---@param index integer
---@param save_path? string
---@return string
function LtFilenames:file_path(index, save_path) end

---@return integer
function LtFilenames:num_files() end

---@return integer
function LtFilenames:num_pieces() end
