---@meta

---`==` is true for two handles to the same session (also once it's gone);
---`tostring` gives `PoSessionHandle(<id>, "<name>")` or `PoSessionHandle(gone)`.
---@class PoSessionHandle
---@field id integer
---@field name string
local PoSessionHandle = {}

---@return PoTorrentsHandle
function PoSessionHandle.torrents() end
