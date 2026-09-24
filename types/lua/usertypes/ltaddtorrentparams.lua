---@meta

---@class LtAddTorrentParams
---@field name string?
---@field save_path string?
LtAddTorrentParams = {}

---@param uri string
---@return LtAddTorrentParams?
---@return string?
function LtAddTorrentParams.from_magnet(uri) end
