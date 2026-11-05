---@meta

---@class PoTorrentsHandle
local PoTorrentsHandle = {}

---@param params LtAddTorrentParams
---@return LtInfoHash?
---@return PoError? err # `session_not_found`, `session_not_loaded`, `torrent_already_added`, `preset_not_found`, `torrent_add_failed` or `invalid_params`
function PoTorrentsHandle:add(params) end

---@param query PoQuery? # An optional query for filtering
---@return fun(): LtTorrentHandle?, LtTorrentStatus?
function PoTorrentsHandle:list(query) end

---@class PoTorrentsRemoveOpts
---@field delete_files? boolean

---@param torrent LtInfoHash|LtTorrentHandle
---@param opts? PoTorrentsRemoveOpts
---@return boolean # false when the torrent isn't in this session
function PoTorrentsHandle:remove(torrent, opts) end
