---@meta

---`==` is true for two handles to the same session (also once it's gone);
---`tostring` gives `PoSessionHandle(<id>, "<name>")` or `PoSessionHandle(gone)`.
---Once the session is gone, every field and method except `==` and `tostring` raises a
---`PoError` with `code = "session_not_loaded"`.
---@class PoSessionHandle
---@field id integer
---@field name string
local PoSessionHandle = {}

---True when the session listens on at least one interface.
---@return boolean
function PoSessionHandle:is_listening() end

---@return PoTorrentsHandle
function PoSessionHandle.torrents() end
