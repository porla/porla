---@meta

---A piece in the download queue; fields are readonly.
---@class LtPartialPieceInfo
---@field piece_index integer # 0-based
---@field blocks_in_piece integer
---@field finished integer # blocks
---@field writing integer # blocks
---@field requested integer # blocks
---@field blocks LtBlockInfo[]
local LtPartialPieceInfo = {}
