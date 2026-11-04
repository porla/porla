---@meta

---@class LtAddTorrentParams
---@field added_time integer?
---@field completed_time integer?
---@field creation_date integer?
---@field last_download integer?
---@field last_seen_complete integer?
---@field last_upload integer?
---@field name string?
---@field save_path string?
LtAddTorrentParams = {}

---@param uri string
---@return LtAddTorrentParams?
---@return string?
function LtAddTorrentParams.from_magnet(uri) end
