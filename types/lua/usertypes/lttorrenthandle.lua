---@meta

---@class LtTorrentHandle
local LtTorrentHandle = {}

---@param url string
function LtTorrentHandle:add_url_seed(url) end

function LtTorrentHandle:clear_error() end

function LtTorrentHandle:clear_peers() end

function LtTorrentHandle:clear_piece_deadlines() end

---@return integer
function LtTorrentHandle:download_limit() end

---@return LtOpenFileState[]
function LtTorrentHandle:file_status() end

---@return LtTorrentFlags
function LtTorrentHandle:flags() end

function LtTorrentHandle:flush_cache() end

function LtTorrentHandle:force_dht_announce() end

function LtTorrentHandle:force_lsd_announce() end

---@class LtReannounceFlagsT
---@field ignore_min_interval? boolean
---@field high_priority? boolean

---@param seconds? integer
---@param index? integer
---@param flags? LtReannounceFlagsT
function LtTorrentHandle:force_reannounce(seconds, index, flags) end

function LtTorrentHandle:force_recheck() end

---@return integer[]
function LtTorrentHandle:get_file_priorities() end

---@return LtPeerInfo[]
function LtTorrentHandle:get_peer_info() end

---@return LtRenamedFiles
function LtTorrentHandle:get_renamed_files() end

---@param index integer
---@return boolean
function LtTorrentHandle:have_piece(index) end

---@return boolean
function LtTorrentHandle:in_session() end

---@return LtInfoHash
function LtTorrentHandle:info_hash() end

---@return boolean
function LtTorrentHandle:is_valid() end

---@return integer
function LtTorrentHandle:max_connections() end

---@return integer
function LtTorrentHandle:max_uploads() end

---@alias LtMoveFlags
---| "always_replace_files"
---| "fail_if_exist"
---| "dont_replace"
---| "reset_save_path"
---| "reset_save_path_unchecked"

---@class LtMoveStorageOpts
---@field flags? LtMoveFlags

---@param path string
---@param opts? LtMoveStorageOpts
function LtTorrentHandle:move_storage(path, opts) end

function LtTorrentHandle:pause() end

function LtTorrentHandle:post_download_queue() end

function LtTorrentHandle:post_file_priorities() end

function LtTorrentHandle:post_file_progress() end

function LtTorrentHandle:post_file_status() end

function LtTorrentHandle:post_peer_info() end

function LtTorrentHandle:post_piece_availability() end

function LtTorrentHandle:post_status() end

function LtTorrentHandle:post_trackers() end

---@param prios integer[]
function LtTorrentHandle:prioritize_files(prios) end

function LtTorrentHandle:queue_position() end

function LtTorrentHandle:queue_position_bottom() end

function LtTorrentHandle:queue_position_down() end

---@param pos integer
function LtTorrentHandle:queue_position_set(pos) end

function LtTorrentHandle:queue_position_top() end

function LtTorrentHandle:queue_position_up() end

---@param index integer
function LtTorrentHandle:read_piece(index) end

---@param url string
function LtTorrentHandle:remove_url_seed(url) end

---@param index integer
---@param name string
function LtTorrentHandle:rename_file(index, name) end

function LtTorrentHandle:resume() end

function LtTorrentHandle:save_resume_data() end

---@param limit integer
function LtTorrentHandle:set_download_limit(limit) end

---@param flags LtTorrentFlags
---@param mask? LtTorrentFlags
function LtTorrentHandle:set_flags(flags, mask) end

---@param n integer
function LtTorrentHandle:set_max_connections(n) end

---@param n integer
function LtTorrentHandle:set_max_uploads(n) end

---@param certificate string
---@param private_key string
---@param dh_params string
---@param passphrase string
function LtTorrentHandle:set_ssl_certificate(certificate, private_key, dh_params, passphrase) end

---@param buffer string
---@param private_key string
---@param dh_params string
---@param passphrase string
function LtTorrentHandle:set_ssl_certificate_buffer(buffer, private_key, dh_params, passphrase) end

---@param limit integer
function LtTorrentHandle:set_upload_limit(limit) end

---@return LtTorrentStatus
function LtTorrentHandle:status() end

---@return LtTorrentInfo
function LtTorrentHandle:torrent_file() end

---@return LtAnnounceEntry[]
function LtTorrentHandle:trackers() end

---@return integer
function LtTorrentHandle:upload_limit() end

---@return string[]
function LtTorrentHandle:url_seeds() end

---@return PoTorrentData
function LtTorrentHandle:userdata() end
