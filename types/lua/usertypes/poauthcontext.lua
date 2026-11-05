---@meta

---@class PoAuthContext
---@field kind "anon"|"machine"|"user" `machine` is an API key, `user` a signed-in user (read-only)
---@field subject? string the username for `user`, the API key id for `machine`, nil for `anon` (read-only)
local PoAuthContext = {}

---@return boolean # true unless `kind` is `anon`
function PoAuthContext:is_authenticated() end
