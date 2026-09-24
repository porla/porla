---@meta

---@class LtTorrentHandle
local LtTorrentHandle = {}

---@return LtInfoHash
function LtTorrentHandle:info_hash() end

---@return LtRenamedFiles
function LtTorrentHandle:get_renamed_files() end

---@return LtTorrentInfo
function LtTorrentHandle:torrent_file() end
