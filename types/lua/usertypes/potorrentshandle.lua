---@meta

---@class PoTorrentsHandle
local PoTorrentsHandle = {}

---@param params LtAddTorrentParams
function PoTorrentsHandle:add(params) end

---@param query PoQuery? # An optional query for filtering
---@return fun(): LtTorrentHandle?, LtTorrentStatus?
function PoTorrentsHandle:list(query) end
