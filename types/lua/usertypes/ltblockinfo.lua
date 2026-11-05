---@meta

---A block of a piece in the download queue; fields are readonly.
---@class LtBlockInfo
---@field state "none"|"requested"|"writing"|"finished"
---@field bytes_progress integer
---@field block_size integer # bytes
---@field num_peers integer
---@field peer [string, integer]? # address and port; nil when no peer has the block
local LtBlockInfo = {}
