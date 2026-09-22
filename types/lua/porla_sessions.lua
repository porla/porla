---@meta porla_sessions

local sessions = {}

---@return integer # The number of sessions
function sessions.count() end

---@return PoSessionHandle? # The default session (if any)
function sessions.default() end

---@param id integer|string # The ID or name of a session
---@return PoSessionHandle? # The session with the given ID/name, or nil.
function sessions.get(id) end

return sessions
