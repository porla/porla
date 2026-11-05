---@meta

---A tracker as seen from one local listen socket; fields are readonly.
---@class LtAnnounceEndpoint
---@field enabled boolean
---@field info_hashes LtAnnounceInfohash[] # `[1]` the v1 announce, `[2]` the v2 announce
---@field local_endpoint [string, integer] # address and port
local LtAnnounceEndpoint = {}
