---@meta

---A plain Lua table, checked only when the torrent is added (`PoTorrentsHandle:add`): a field of
---the wrong type is ignored, while a bad entry in `file_priorities`, `trackers`, `tracker_tiers`,
---`url_seeds` or `userdata` raises a `PoError` with `code = "invalid_params"`. A table from `from_*`
---also carries what it has no field for (peers, renamed files, ...), and its `download_limit`,
---`flags`, `max_connections`, `max_uploads`, `save_path`, `storage_mode` and `upload_limit` override
---the preset only when changed. Times are unix seconds, nil when unset; durations are seconds.
---@class LtAddTorrentParams
---@field active_time? integer # seconds
---@field added_time? integer
---@field comment? string
---@field completed_time? integer
---@field created_by? string
---@field creation_date? integer
---@field download_limit? integer # bytes/s; -1 for unlimited
---@field file_priorities? integer[] # 0-7 per file from the first
---@field finished_time? integer # seconds
---@field flags? LtTorrentFlags
---@field info_hash? LtInfoHash
---@field last_download? integer
---@field last_seen_complete? integer
---@field last_upload? integer
---@field max_connections? integer
---@field max_uploads? integer
---@field name? string
---@field num_complete? integer
---@field num_downloaded? integer
---@field num_incomplete? integer
---@field part_file_dir? string
---@field root_certificate? string
---@field save_path? string
---@field seeding_time? integer # seconds
---@field storage_mode? "allocate"|"sparse" # any other string is "sparse"
---@field ti? LtTorrentInfo
---@field trackerid? string
---@field tracker_tiers? integer[] # 0 or more per tracker, no more entries than `trackers`
---@field trackers? string[]
---@field total_downloaded? integer
---@field total_uploaded? integer
---@field upload_limit? integer # bytes/s; -1 for unlimited
---@field url_seeds? string[]
---@field userdata? LtAddTorrentParamsUserdata # never set on a table from `from_*`
LtAddTorrentParams = {}

---@class LtAddTorrentParamsUserdata
---@field category? string
---@field metadata? table<string, any> # must encode as a JSON object
---@field tags? string[]

---@param buffer string # contents of a .torrent file
---@return LtAddTorrentParams? params
---@return PoError? err # `invalid_params`
function LtAddTorrentParams.from_buffer(buffer) end

---@param uri string
---@return LtAddTorrentParams? params
---@return PoError? err # `invalid_params`
function LtAddTorrentParams.from_magnet(uri) end

---@param path string # path to a .torrent file
---@return LtAddTorrentParams? params
---@return PoError? err # `invalid_params`
function LtAddTorrentParams.from_path(path) end
