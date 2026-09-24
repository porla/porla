---@meta

---@class LtTorrentStatus
---@field added_time integer
---@field all_time_download integer
---@field all_time_upload integer
---@field download_payload_rate integer
---@field errc PoError?
---@field flags LtTorrentFlags
---@field info_hash LtInfoHash
---@field moving_storage boolean
---@field name string
---@field progress number
---@field save_path string
---@field seeding_duration integer
---@field state string?
---@field total_wanted integer
---@field total_wanted_done integer
---@field upload_payload_rate integer
local LtTorrentStatus = {}
