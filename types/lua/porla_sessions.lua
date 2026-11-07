---@meta porla_sessions

local sessions = {}

---@return integer # The number of loaded sessions; sessions that failed to load aren't counted
function sessions.count() end

---@return PoSessionHandle? # The default session, or nil if there is none or it isn't loaded
function sessions.default() end

---@param id integer
---@return PoSessionHandle? # nil when no loaded session has this id
---@overload fun(name: string): PoSessionHandle?
function sessions.get(id) end

---The loaded sessions in id order: `for _, session in ipairs(sessions.list()) do ... end`.
---@return PoSessionHandle[]
function sessions.list() end

---@class PoSessionMetric
---@field type "counter"|"gauge"

---Every libtorrent session metric, keyed by name (e.g. `net.sent_payload_bytes`); the same keys as `stats` in `session.stats` events.
---@return table<string, PoSessionMetric>
function sessions.metrics() end

return sessions
