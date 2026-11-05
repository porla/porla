---@meta

---A torrent's metadata.
---@class LtTorrentInfo
local LtTorrentInfo = {}

---@return LtInfoHash
function LtTorrentInfo:info_hash() end

---Raises a `PoError` with `code = "torrent_no_metadata"` for a torrent without metadata.
---@return LtFileStorage
function LtTorrentInfo:layout() end

---@return string
function LtTorrentInfo:name() end

---@return integer
function LtTorrentInfo:num_files() end

---@return integer
function LtTorrentInfo:num_pieces() end

---@return boolean
function LtTorrentInfo:priv() end

---@return integer # bytes
function LtTorrentInfo:total_size() end
