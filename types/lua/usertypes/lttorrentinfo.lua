---@meta
---@class LtTorrentInfo
local LtTorrentInfo = {}

---@return LtInfoHash
function LtTorrentInfo:info_hash() end

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

---@return integer
function LtTorrentInfo:total_size() end
