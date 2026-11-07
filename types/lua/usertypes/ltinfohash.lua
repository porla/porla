---@meta

---`==` compares both halves; `<` orders hashes; `tostring` is `hex()`. Constructing from a string
---that isn't 40 (v1) or 64 (v2) hex characters raises a `PoError` with `code = "invalid_params"`.
---@class LtInfoHash
---@overload fun(hex: string): LtInfoHash
---@overload fun(hex_v1: string, hex_v2: string): LtInfoHash # a hybrid torrent's hashes
---@field v1 string? # hex; readonly
---@field v2 string? # hex; readonly
LtInfoHash = {}

---The v1 hash as hex, or the v2 hash for a v2-only torrent. Use it to key tables by torrent;
---a v2-only magnet's key changes to its v1 hash once it has metadata. To look a torrent up again, keep the
---`LtInfoHash` itself (or both `v1` and `v2`): `LtInfoHash(ih:hex())` has only one half, so `torrents:get` and
---`torrents:remove` don't find a hybrid torrent with it.
---@return string
function LtInfoHash:hex() end
