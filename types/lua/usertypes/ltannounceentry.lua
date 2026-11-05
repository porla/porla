---@meta

---A tracker of a torrent; fields are readonly.
---@class LtAnnounceEntry
---@field endpoints LtAnnounceEndpoint[] # one per local listen socket
---@field fail_limit integer # failures before giving up; 0 for no limit
---@field source integer # bitmask: 1 torrent file, 2 added by a client, 4 magnet link, 8 tracker exchange
---@field tier integer
---@field trackerid string
---@field url string
---@field verified boolean
local LtAnnounceEntry = {}
