---@meta

---The torrents of one session. Once the session is unloaded, `add` fails, `count` returns -1, `get`
---returns nil, `list` returns nil and `remove` returns false.
---@class PoTorrentsHandle
local PoTorrentsHandle = {}

---Callable for a generic `for`; walks torrents in info hash order and copes with torrents being
---added or removed during the loop. Ends early if the session is unloaded.
---@alias PoTorrentsIterator fun(): LtTorrentHandle?, LtTorrentStatus?

---@class PoTorrentsHandleAddOpts
---@field preset_id? integer # Takes precedence over `preset`
---@field preset? string

---Raises `invalid_params` for a malformed field in `params`.
---@param params LtAddTorrentParams
---@param opts? PoTorrentsHandleAddOpts
---@return LtInfoHash?
---@return PoError|string|nil err # `session_not_found`, `session_not_loaded`, `torrent_already_added`, `preset_not_found`, `torrent_add_failed` or `invalid_params` (missing info hash or save path, invalid UTF-8); the string `"Session not loaded"` when this handle's session is gone
function PoTorrentsHandle:add(params, opts) end

---@return integer # -1 once the session is unloaded
function PoTorrentsHandle:count() end

---The status is the cached one from the last status update.
---@param info_hash LtInfoHash
---@return LtTorrentHandle? # nil when the torrent isn't in this session
---@return LtTorrentStatus?
function PoTorrentsHandle:get(info_hash) end

---Only torrents matching `query`. Passing nil raises; call `list()` instead.
---@param query PoQuery
---@return PoTorrentsIterator?
---@overload fun(self: PoTorrentsHandle): PoTorrentsIterator?
function PoTorrentsHandle:list(query) end

---@class PoTorrentsHandleRemoveOpts
---@field delete_files? boolean # Raises `invalid_params` when not a boolean

---Asks libtorrent to remove the torrent; it is gone once `torrent.removed` fires.
---@param torrent LtInfoHash|LtTorrentHandle
---@param opts? PoTorrentsHandleRemoveOpts
---@return boolean # false when the torrent isn't in this session or the handle is invalid
function PoTorrentsHandle:remove(torrent, opts) end
