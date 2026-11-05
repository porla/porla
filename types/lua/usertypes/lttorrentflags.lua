---@meta

---@alias LtTorrentFlag
---| "seed_mode"
---| "upload_mode"
---| "share_mode"
---| "apply_ip_filter"
---| "paused"
---| "auto_managed"
---| "duplicate_is_error"
---| "update_subscribe"
---| "super_seeding"
---| "sequential_download"
---| "stop_when_ready"
---| "need_save_resume"
---| "disable_dht"
---| "disable_lsd"
---| "disable_pex"
---| "no_verify_files"
---| "default_dont_download"
---| "i2p_torrent"
---| "disable_v1_hashes"

---A set of torrent flags; `LtTorrentFlags()` is empty. An unknown flag name raises a `PoError`
---with `code = "invalid_params"`. `==` compares the sets; `tostring` gives the names joined by
---`|` in alphabetical order, or `(none)`.
---@class LtTorrentFlags
---@overload fun(): LtTorrentFlags
LtTorrentFlags = {}

---@param flag LtTorrentFlag
---@return boolean
function LtTorrentFlags:has(flag) end

---@param flag LtTorrentFlag
function LtTorrentFlags:set(flag) end

---@param flag LtTorrentFlag
function LtTorrentFlags:unset(flag) end
