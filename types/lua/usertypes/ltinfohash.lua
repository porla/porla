---@meta

---`==` compares both halves; `tostring` is `hex()`.
---@class LtInfoHash
---@field v1 string?
---@field v2 string?
LtInfoHash = {}

---The v1 hash as hex, or the v2 hash for a v2-only torrent. Use it to key tables by torrent;
---a v2-only magnet's key changes to its v1 hash once it has metadata.
---@return string
function LtInfoHash:hex() end
