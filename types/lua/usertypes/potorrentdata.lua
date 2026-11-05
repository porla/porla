---@meta

---Porla's own data for a torrent; every read sees the current value. Fields are read-only, change
---them with `update`. Once the torrent is removed, every field and method raises a `PoError` with
---`code = "torrent_not_found"`.
---@class PoTorrentData
---@field category string? # nil when the torrent has no category
---@field session PoSessionHandle
local PoTorrentData = {}

---@param tag string
---@return boolean
function PoTorrentData:has_tag(tag) end

---Nested JSON nulls come back as `porla.null`.
---@param key string
---@return any # nil when `key` isn't set
function PoTorrentData:metadata(key) end

---@return string[] # In no particular order
function PoTorrentData:tags() end

---@class PoTorrentDataChanges
---@field category? string|PoJsonNull # `porla.null` clears the category
---@field tags? string[] # Replaces the whole tag set
---@field metadata? table<string, any> # Merged into the existing metadata; a `porla.null` value removes that key

---Applies and persists all changes at once; fields left out (or nil) are kept. Raises `invalid_params`
---for a wrong type or invalid UTF-8, and `torrent_not_found` once the torrent is removed.
---@param changes PoTorrentDataChanges
function PoTorrentData:update(changes) end
