---@meta

---Methods raise a `PoError`: `torrent_not_found` once the torrent is removed (except `is_valid`,
---`info_hash`, `torrent_file`, `file_status` and `userdata`), `invalid_params` for bad arguments,
---`torrent_no_metadata` where metadata is needed. Getters marked "waits for libtorrent" block until
---libtorrent answers; each `post_*` method asks for the same data without waiting and answers with an
---event. Indices are 0-based. `==` is true for handles to the same torrent; `<` orders handles.
---@class LtTorrentHandle
local LtTorrentHandle = {}

---@class LtTrackerOpts
---@field tier? integer # 0-255

---@param url string # must not be empty
---@param opts? LtTrackerOpts
function LtTorrentHandle:add_tracker(url, opts) end

---@param url string
function LtTorrentHandle:add_url_seed(url) end

function LtTorrentHandle:clear_error() end

function LtTorrentHandle:clear_peers() end

function LtTorrentHandle:clear_piece_deadlines() end

---@param ip string # IPv4 or IPv6 address
---@param port integer # 1-65535
function LtTorrentHandle:connect_peer(ip, port) end

---Waits for libtorrent.
---@return integer # bytes/s; 0 or -1 when unlimited
function LtTorrentHandle:download_limit() end

---Exact bytes downloaded per file; entry `i + 1` is file `i`. Waits for libtorrent, like `status()`.
---@return integer[]
function LtTorrentHandle:file_progress() end

---Files libtorrent has open; empty once the torrent is removed.
---@return LtOpenFileState[]
function LtTorrentHandle:file_status() end

---Waits for libtorrent.
---@return LtTorrentFlags
function LtTorrentHandle:flags() end

function LtTorrentHandle:flush_cache() end

function LtTorrentHandle:force_dht_announce() end

function LtTorrentHandle:force_lsd_announce() end

---@class LtReannounceFlagsT
---@field ignore_min_interval? boolean
---@field high_priority? boolean

---Waits for libtorrent to check `index`.
---
---**Warning:** `index` is 0-based but `trackers()` is a 1-based Lua array; inside
---`for i, t in ipairs(th:trackers())` pass `i - 1`.
---@param seconds? integer # delay before announcing; 0 when omitted
---@param index? integer # 0-based tracker index; -1 or omitted for every tracker
---@param flags? LtReannounceFlagsT
function LtTorrentHandle:force_reannounce(seconds, index, flags) end

function LtTorrentHandle:force_recheck() end

---Waits for libtorrent.
---@return LtPartialPieceInfo[]
function LtTorrentHandle:get_download_queue() end

---Priority (0-7) per file; entry `i + 1` is file `i`. Waits for libtorrent.
---@return integer[]
function LtTorrentHandle:get_file_priorities() end

---Waits for libtorrent.
---@return LtPeerInfo[]
function LtTorrentHandle:get_peer_info() end

---Priority (0-7) per piece; entry `i + 1` is piece `i`. Waits for libtorrent.
---@return integer[]
function LtTorrentHandle:get_piece_priorities() end

---Waits for libtorrent.
---@return LtRenamedFiles
function LtTorrentHandle:get_renamed_files() end

---Waits for libtorrent.
---@param index integer
---@return boolean # false for an index out of range or without metadata
function LtTorrentHandle:have_piece(index) end

---Waits for libtorrent.
---@return boolean
function LtTorrentHandle:in_session() end

---@return LtInfoHash # empty (`v1` and `v2` nil) once the torrent is removed
function LtTorrentHandle:info_hash() end

---@return boolean
function LtTorrentHandle:is_valid() end

---Waits for libtorrent.
---@return integer # 16777215 when unlimited
function LtTorrentHandle:max_connections() end

---Waits for libtorrent.
---@return integer # 16777215 when unlimited
function LtTorrentHandle:max_uploads() end

---@alias LtMoveFlags
---| "always_replace_files"
---| "fail_if_exist"
---| "dont_replace"
---| "reset_save_path"
---| "reset_save_path_unchecked"

---@class LtMoveStorageOpts
---@field flags? LtMoveFlags # "dont_replace" when omitted

---Answered by `torrent.storage_moved` or `torrent.storage_moved_failed`.
---@param path string
---@param opts? LtMoveStorageOpts
function LtTorrentHandle:move_storage(path, opts) end

function LtTorrentHandle:pause() end

---Peers that have each piece; entry `i + 1` is piece `i`. Empty without metadata. Waits for libtorrent.
---@return integer[]
function LtTorrentHandle:piece_availability() end

---Returns the piece's priority (0-7); waits for libtorrent. With `priority` (0-7), sets it instead.
---@param index integer
---@return integer
---@overload fun(self: LtTorrentHandle, index: integer, priority: integer)
function LtTorrentHandle:piece_priority(index) end

---Answered by `torrent.piece_info`.
function LtTorrentHandle:post_download_queue() end

---Answered by `torrent.file_priorities`.
function LtTorrentHandle:post_file_priorities() end

---Answered by `torrent.file_progress`, rounded down to whole pieces (unlike `file_progress()`).
function LtTorrentHandle:post_file_progress() end

---Answered by `torrent.file_status`.
function LtTorrentHandle:post_file_status() end

---Answered by `torrent.peer_info`.
function LtTorrentHandle:post_peer_info() end

---Answered by `torrent.piece_availability`.
function LtTorrentHandle:post_piece_availability() end

---Answered by `torrent.status`.
function LtTorrentHandle:post_status() end

---Answered by `torrent.trackers`.
function LtTorrentHandle:post_trackers() end

---@param prios integer[] # 0-7 per file from the first
function LtTorrentHandle:prioritize_files(prios) end

---@param prios integer[] # 0-7 per piece from the first; fewer than the torrent's pieces leaves the rest as they are
function LtTorrentHandle:prioritize_pieces(prios) end

---Waits for libtorrent.
---@return integer # -1 when not queued
function LtTorrentHandle:queue_position() end

function LtTorrentHandle:queue_position_bottom() end

function LtTorrentHandle:queue_position_down() end

---@param pos integer # 0 or more
function LtTorrentHandle:queue_position_set(pos) end

function LtTorrentHandle:queue_position_top() end

function LtTorrentHandle:queue_position_up() end

---Answered by the `torrent.piece_read` event, which carries the error for a piece out of range,
---not downloaded yet, or without metadata.
---@param index integer
function LtTorrentHandle:read_piece(index) end

---@param url string
function LtTorrentHandle:remove_url_seed(url) end

---@param index integer
---@param name string # must not be empty
function LtTorrentHandle:rename_file(index, name) end

---@param trackers (string|{ url: string, tier?: integer })[] # tier 0-255; urls must not be empty
function LtTorrentHandle:replace_trackers(trackers) end

---@param index integer
function LtTorrentHandle:reset_piece_deadline(index) end

function LtTorrentHandle:resume() end

---Includes the info dict and flushes the disk cache first.
function LtTorrentHandle:save_resume_data() end

---Scrape now: every tracker, the tracker at `index` (waits for libtorrent), or the tracker with this url.
---
---**Warning:** `index` is 0-based but `trackers()` is a 1-based Lua array, so `scrape_tracker(i)` inside
---`for i, t in ipairs(th:trackers())` scrapes the next tracker and raises on the last one. Pass `t.url`
---instead, or `i - 1`.
---@param tracker? integer|string
function LtTorrentHandle:scrape_tracker(tracker) end

---@param limit integer # bytes/s; -1 for unlimited, below -1 raises
function LtTorrentHandle:set_download_limit(limit) end

---Sets the flags in `flags`; with `mask`, sets each flag in `mask` to its value in `flags`.
---@param flags LtTorrentFlags
---@param mask? LtTorrentFlags
function LtTorrentHandle:set_flags(flags, mask) end

---@param n integer # 2 or more; 0 or -1 for unlimited
function LtTorrentHandle:set_max_connections(n) end

---@param n integer # 2 or more; 0 or -1 for unlimited
function LtTorrentHandle:set_max_uploads(n) end

---@class LtPieceDeadlineOpts
---@field alert_when_available? boolean # read the piece once it's downloaded (torrent.piece_read)

---@param index integer
---@param seconds number # 0 or more, to the millisecond
---@param opts? LtPieceDeadlineOpts
function LtTorrentHandle:set_piece_deadline(index, seconds, opts) end

---Download pieces `first` to `last` in order (sequential mode); without `last`, to the end of the file `first` is in.
---@param first integer
---@param last? integer # not before `first`
function LtTorrentHandle:set_sequential_range(first, last) end

---Paths to PEM files, for an SSL torrent (`torrent.need_cert`).
---@param certificate string
---@param private_key string
---@param dh_params string
---@param passphrase? string
function LtTorrentHandle:set_ssl_certificate(certificate, private_key, dh_params, passphrase) end

---Like `set_ssl_certificate`, with the PEM contents instead of paths and no passphrase.
---@param certificate string
---@param private_key string
---@param dh_params string
function LtTorrentHandle:set_ssl_certificate_buffer(certificate, private_key, dh_params) end

---@param limit integer # bytes/s; -1 for unlimited, below -1 raises
function LtTorrentHandle:set_upload_limit(limit) end

---Waits for libtorrent.
---@return LtTorrentStatus
function LtTorrentHandle:status() end

---@return LtTorrentInfo? # nil without metadata or once the torrent is removed
function LtTorrentHandle:torrent_file() end

---Waits for libtorrent.
---@return LtAnnounceEntry[]
function LtTorrentHandle:trackers() end

---@param flags LtTorrentFlags
function LtTorrentHandle:unset_flags(flags) end

---Waits for libtorrent.
---@return integer # bytes/s; 0 or -1 when unlimited
function LtTorrentHandle:upload_limit() end

---Sorted. Waits for libtorrent.
---@return string[]
function LtTorrentHandle:url_seeds() end

---@return PoTorrentData? # nil once the torrent is removed
function LtTorrentHandle:userdata() end
