---@meta porla_events

---Every event table has `name`. Session-bound events add `session_id` and `session`, torrent events add
---`info_hash` and `torrent`. Events are delivered after the change, so `session` and `torrent` are nil
---when the session or torrent is gone by then. `error` fields are `libtorrent_error` PoErrors with `value` set.
---@class PoEvent
---@field name string

---@class PoSessionEvent: PoEvent
---@field session_id integer
---@field session? PoSessionHandle

---@class PoTorrentEvent: PoSessionEvent
---@field info_hash LtInfoHash
---@field torrent? LtTorrentHandle

---@class PoKvUpdatedEvent: PoEvent
---@field keys string[] the keys written, even if the value didn't change; read-only keys are left out

---A plugin never receives `plugin.*` events about itself.
---@class PoPluginEvent: PoEvent
---@field plugin_id integer

---@class PoPluginRemovedEvent: PoPluginEvent
---@field plugin_path string where the plugin was loaded from

---@class PoPresetEvent: PoEvent
---@field preset_id integer

---@class PoPresetRemovedEvent: PoPresetEvent
---@field preset_name string

---The session no longer exists, so there is no `session` field.
---@class PoSessionGoneEvent: PoEvent
---@field session_id integer
---@field session_name string

---@class PoSessionLoadedEvent: PoSessionEvent
---@field torrents integer the number of torrents loaded
---@field failed boolean true if loading stopped early because of an error

---@class PoSessionStatsEvent: PoSessionEvent
---@field stats table<string, integer> keyed by metric name, see `porla_sessions.metrics()`

---@class PoDhtLookup
---@field type string
---@field outstanding_requests integer
---@field timeouts integer
---@field responses integer
---@field branch_factor integer
---@field nodes_left integer
---@field last_sent integer seconds ago
---@field first_timeout integer
---@field target string hex

---@class PoDhtRoutingBucket
---@field num_nodes integer
---@field num_replacements integer
---@field last_active integer seconds ago

---@class PoDhtStats
---@field active_requests PoDhtLookup[]
---@field routing_table PoDhtRoutingBucket[]
---@field nid string node id, hex
---@field local_endpoint [string, integer] address and port of the DHT node

---@class PoSessionDhtStatsEvent: PoSessionEvent
---@field stats PoDhtStats

---`torrent` is always nil.
---@class PoTorrentAddFailedEvent: PoTorrentEvent
---@field loading boolean true when the torrent failed to load from Porla's database, false when it was being added
---@field error PoError
---@field torrent_name? string from the torrent file or magnet link, when it has one

---@class PoTorrentStatusEvent: PoTorrentEvent
---@field status LtTorrentStatus

---@class PoTorrentMetadataReceivedEvent: PoTorrentEvent
---@field torrent_file LtTorrentInfo

---@class PoTorrentErrorEvent: PoTorrentEvent
---@field error PoError
---@field filename string the file involved, empty if none

---@class PoTorrentFileErrorEvent: PoTorrentEvent
---@field error PoError
---@field filename string
---@field op string the operation that failed, e.g. `file_open`

---@class PoTorrentFilePrioritiesEvent: PoTorrentEvent
---@field priorities integer[] per file, 0-7

---@class PoTorrentFileProgressEvent: PoTorrentEvent
---@field progress integer[] bytes downloaded per file, counting only complete pieces

---@class PoTorrentFileStatusEvent: PoTorrentEvent
---@field files LtOpenFileState[] the files libtorrent has open

---@class PoTorrentFinishedEvent: PoTorrentEvent
---@field first boolean true only the first time the torrent finishes after downloading payload; false for rechecks and restarts

---@class PoTorrentPeerInfoEvent: PoTorrentEvent
---@field peers LtPeerInfo[]

---@class PoTorrentPieceAvailabilityEvent: PoTorrentEvent
---@field availability integer[] per piece, the number of peers that have it

---@class PoTorrentPieceInfoEvent: PoTorrentEvent
---@field pieces LtPartialPieceInfo[] the pieces being downloaded

---@class PoTorrentPieceReadEvent: PoTorrentEvent
---@field piece integer
---@field error? PoError nil on success
---@field data? string the piece's bytes, nil on error

---@alias PoTorrentState "checking_files"|"downloading_metadata"|"downloading"|"finished"|"seeding"|"checking_resume_data"

---@class PoTorrentStateChangedEvent: PoTorrentEvent
---@field state PoTorrentState
---@field prev_state PoTorrentState

---@class PoTorrentStorageMovedEvent: PoTorrentEvent
---@field storage_path string
---@field old_path string

---@class PoTorrentStorageMovedFailedEvent: PoTorrentEvent
---@field error PoError
---@field file_path string the file involved, empty if none
---@field op string the operation that failed

---@class PoTrackerEvent: PoTorrentEvent
---@field tracker_url string
---@field local_endpoint [string, integer] local address and port used for the announce

---Not emitted for skipped announces.
---@class PoTorrentTrackerErrorEvent: PoTrackerEvent
---@field times_in_row integer
---@field error PoError
---@field op string the operation that failed
---@field failure_reason string from the tracker, empty if none

---@class PoTorrentTrackerReplyEvent: PoTrackerEvent
---@field num_peers integer

---@class PoTorrentTrackerWarningEvent: PoTrackerEvent
---@field warning_message string

---@class PoTorrentTrackersEvent: PoTorrentEvent
---@field trackers LtAnnounceEntry[]

---@class PoTorrentUserdataUpdatedEvent: PoTorrentEvent
---@field fields ("category"|"metadata"|"tags")[] which fields changed

---@alias PoEventName
---| "kv.updated" # one or more keys were written
---| "plugin.added" # `plugin.loaded` follows if it loads
---| "plugin.loaded"
---| "plugin.removed"
---| "plugin.unloaded" # on reload, update or removal
---| "plugin.updated" # its settings changed; `plugin.unloaded` and `plugin.loaded` follow
---| "preset.added"
---| "preset.removed"
---| "preset.updated" # also for the old default when another preset becomes default, and for every preset of a removed session
---| "session.added"
---| "session.dht_stats" # every `timer_dht_stats` ms and after `post_dht_stats()`, once per DHT node; not while the DHT is off
---| "session.loaded" # the session finished loading its torrents, at startup and when added
---| "session.removed"
---| "session.stats" # every `timer_session_stats` ms and after `post_session_stats()`
---| "session.unloaded"
---| "session.updated" # renamed, made default, timers changed
---| "torrent.add_failed" # libtorrent refused the torrent
---| "torrent.added" # not for torrents loaded from Porla's database at startup
---| "torrent.checked"
---| "torrent.error"
---| "torrent.file_error"
---| "torrent.file_priorities" # reply to `torrent:post_file_priorities()`
---| "torrent.file_progress" # reply to `torrent:post_file_progress()`
---| "torrent.file_status" # reply to `torrent:post_file_status()`
---| "torrent.finished" # every time the torrent becomes finished, including after rechecks; see `first`
---| "torrent.metadata_received" # once, for a torrent added without metadata (a magnet link)
---| "torrent.need_cert" # call `torrent:set_ssl_certificate(...)` from the handler
---| "torrent.peer_info" # reply to `torrent:post_peer_info()`
---| "torrent.piece_availability" # reply to `torrent:post_piece_availability()`
---| "torrent.piece_info" # reply to `torrent:post_download_queue()`
---| "torrent.piece_read" # reply to `torrent:read_piece(i)`
---| "torrent.status" # reply to `torrent:post_status()`, with the refreshed status
---| "torrent.storage_moved" # reply to `torrent:move_storage(...)`
---| "torrent.storage_moved_failed"
---| "torrent.paused"
---| "torrent.removed" # `torrent` is always nil
---| "torrent.resumed"
---| "torrent.state_changed"
---| "torrent.tracker_error"
---| "torrent.tracker_reply"
---| "torrent.tracker_warning"
---| "torrent.trackers" # reply to `torrent:post_trackers()`
---| "torrent.userdata_updated" # category, metadata or tags changed

local events = {}

---Calls `callback` with an event table every time `name` is emitted, until cancelled or the plugin unloads.
---The callback never runs inside this call. Errors it raises are logged and the handler stays registered.
---Raises `invalid_params` for an unknown event name.
---@param name PoEventName
---@param callback fun(event: PoEvent)
---@return PoCancellable
---@overload fun(name: "kv.updated", callback: fun(event: PoKvUpdatedEvent)): PoCancellable
---@overload fun(name: "plugin.added", callback: fun(event: PoPluginEvent)): PoCancellable
---@overload fun(name: "plugin.loaded", callback: fun(event: PoPluginEvent)): PoCancellable
---@overload fun(name: "plugin.removed", callback: fun(event: PoPluginRemovedEvent)): PoCancellable
---@overload fun(name: "plugin.unloaded", callback: fun(event: PoPluginEvent)): PoCancellable
---@overload fun(name: "plugin.updated", callback: fun(event: PoPluginEvent)): PoCancellable
---@overload fun(name: "preset.added", callback: fun(event: PoPresetEvent)): PoCancellable
---@overload fun(name: "preset.removed", callback: fun(event: PoPresetRemovedEvent)): PoCancellable
---@overload fun(name: "preset.updated", callback: fun(event: PoPresetEvent)): PoCancellable
---@overload fun(name: "session.added", callback: fun(event: PoSessionEvent)): PoCancellable
---@overload fun(name: "session.dht_stats", callback: fun(event: PoSessionDhtStatsEvent)): PoCancellable
---@overload fun(name: "session.loaded", callback: fun(event: PoSessionLoadedEvent)): PoCancellable
---@overload fun(name: "session.removed", callback: fun(event: PoSessionGoneEvent)): PoCancellable
---@overload fun(name: "session.stats", callback: fun(event: PoSessionStatsEvent)): PoCancellable
---@overload fun(name: "session.unloaded", callback: fun(event: PoSessionGoneEvent)): PoCancellable
---@overload fun(name: "session.updated", callback: fun(event: PoSessionEvent)): PoCancellable
---@overload fun(name: "torrent.add_failed", callback: fun(event: PoTorrentAddFailedEvent)): PoCancellable
---@overload fun(name: "torrent.added", callback: fun(event: PoTorrentStatusEvent)): PoCancellable
---@overload fun(name: "torrent.checked", callback: fun(event: PoTorrentEvent)): PoCancellable
---@overload fun(name: "torrent.error", callback: fun(event: PoTorrentErrorEvent)): PoCancellable
---@overload fun(name: "torrent.file_error", callback: fun(event: PoTorrentFileErrorEvent)): PoCancellable
---@overload fun(name: "torrent.file_priorities", callback: fun(event: PoTorrentFilePrioritiesEvent)): PoCancellable
---@overload fun(name: "torrent.file_progress", callback: fun(event: PoTorrentFileProgressEvent)): PoCancellable
---@overload fun(name: "torrent.file_status", callback: fun(event: PoTorrentFileStatusEvent)): PoCancellable
---@overload fun(name: "torrent.finished", callback: fun(event: PoTorrentFinishedEvent)): PoCancellable
---@overload fun(name: "torrent.metadata_received", callback: fun(event: PoTorrentMetadataReceivedEvent)): PoCancellable
---@overload fun(name: "torrent.need_cert", callback: fun(event: PoTorrentEvent)): PoCancellable
---@overload fun(name: "torrent.peer_info", callback: fun(event: PoTorrentPeerInfoEvent)): PoCancellable
---@overload fun(name: "torrent.piece_availability", callback: fun(event: PoTorrentPieceAvailabilityEvent)): PoCancellable
---@overload fun(name: "torrent.piece_info", callback: fun(event: PoTorrentPieceInfoEvent)): PoCancellable
---@overload fun(name: "torrent.piece_read", callback: fun(event: PoTorrentPieceReadEvent)): PoCancellable
---@overload fun(name: "torrent.status", callback: fun(event: PoTorrentStatusEvent)): PoCancellable
---@overload fun(name: "torrent.storage_moved", callback: fun(event: PoTorrentStorageMovedEvent)): PoCancellable
---@overload fun(name: "torrent.storage_moved_failed", callback: fun(event: PoTorrentStorageMovedFailedEvent)): PoCancellable
---@overload fun(name: "torrent.paused", callback: fun(event: PoTorrentEvent)): PoCancellable
---@overload fun(name: "torrent.removed", callback: fun(event: PoTorrentEvent)): PoCancellable
---@overload fun(name: "torrent.resumed", callback: fun(event: PoTorrentEvent)): PoCancellable
---@overload fun(name: "torrent.state_changed", callback: fun(event: PoTorrentStateChangedEvent)): PoCancellable
---@overload fun(name: "torrent.tracker_error", callback: fun(event: PoTorrentTrackerErrorEvent)): PoCancellable
---@overload fun(name: "torrent.tracker_reply", callback: fun(event: PoTorrentTrackerReplyEvent)): PoCancellable
---@overload fun(name: "torrent.tracker_warning", callback: fun(event: PoTorrentTrackerWarningEvent)): PoCancellable
---@overload fun(name: "torrent.trackers", callback: fun(event: PoTorrentTrackersEvent)): PoCancellable
---@overload fun(name: "torrent.userdata_updated", callback: fun(event: PoTorrentUserdataUpdatedEvent)): PoCancellable
function events.on(name, callback) end

return events
