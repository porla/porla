---@meta porla_timers

local timers = {}

---Runs `callback` on a cron schedule in the server's local time. Six fields, seconds first:
---`sec min hour dom month dow` (croncpp syntax), e.g. `"*/5 * * * * ?"`. Raises `invalid_params` for an invalid expression.
---The schedule keeps running until cancelled or the plugin unloads, even if the returned handle is dropped.
---@param expression string
---@param callback fun()
---@return PoCancellable
function timers.cron(expression, callback) end

---@return integer # The current Unix time in milliseconds
function timers.epoch() end

---Runs `callback` every `ms` milliseconds, the first time after `ms`. Raises `invalid_params` if `ms` is below 10.
---Keeps running until cancelled or the plugin unloads, even if the returned handle is dropped.
---@param ms integer
---@param callback fun()
---@return PoCancellable
function timers.interval(ms, callback) end

---Runs `callback` once after `ms` milliseconds. Raises `invalid_params` if `ms` is below 10.
---@param ms integer
---@param callback fun()
---@return PoCancellable
function timers.timeout(ms, callback) end

return timers
